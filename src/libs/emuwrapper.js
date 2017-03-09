// For some reason, that I really did not want to look into, I
// a) couldn't get emscripten to play nice with webpack
// b) couldn't figure out why repeated call to my assembler function failed

// This is my ghetto solution. Reinit the entire c++ program on each call.

// The following code is bad, AND I feel bad. Please no judge :(

function p_throttle(fn, threshhold = 250) {
  let last, deferTimer;
  return async function (...args) {
    const now = +new Date();
    if (last && now < last + threshhold) {
      clearTimeout(deferTimer);
      return new Promise(resolve => {
        deferTimer = setTimeout(() => {
          last = now;
          resolve(fn.apply(this, args));
        }, threshhold);
      });
    } else {
      last = now;
      return fn.apply(this, args);
    }
  };
}

const getCPP = p_throttle(function() {
  return new Promise(resolve => {
    // Kill any running c++
    if (window.Module) {
      try {
        window.Module.exit(1);
      } catch (e) {
      } finally {
        // why? dunno, but it prevents errors ;)
        window.Module = { monitorRunDependencies: false };
      }
    }

    // reload the ol' scriptaroo
    const oldTag = document.getElementById("emscriptendCode");
    if (oldTag) oldTag.remove();

    const sc = document.createElement("script");
    sc.setAttribute("id", "emscriptendCode");
    sc.setAttribute("src", "mips241.js");
    sc.setAttribute("type", "text/javascript");
    document.head.appendChild(sc);

    // ...and poll for when it's initialized
    // (inb4 "you shouldn't be allowed to touch a computer")
    (function neverdothis() {
      if (!window.Module || !window.Module.MIPS$$RAM) {
        setTimeout(neverdothis, 100);
        return;
      } else {
        resolve(window.Module);
      }
    })();
  });
}, 250);

function processError(err) {
  console.log(err);

  // this shouldn't happen, but alas, it probably will ¯\_(ツ)_/¯
  if (typeof err === "number") {
    return {
      row: 0,
      column: 0,
      type: 'error',
      text: "Unknown error, could not assemble ¯\\_(ツ)_/¯"
    };
  }

  // This error handling matches the errors thrown by my assembler.
  // Your milage will vary.

  // first of all, if the errors are ones I wrote, then they would have the line
  // numbers of the problematic statements. Life is easy.
  const m = err.match(/line (\d+)/);

  if (m && m[1]) {
    return {
      row: parseInt(m[1], 10) - 1,
      column: 0,
      type: 'error',
      text: err
    };
  }

  // If all else fails...
  return {
    row: 0,
    column: 0,
    type: 'error',
    text: err
  };
}

export async function assemble(asm: string) : Array<number> {
  const cpp = await getCPP();
  try {
    return cpp.MIPS$$asm(asm)
      .split(" ")
      .filter((x, i, a) => i !== a.length - 1) // drop empty last element
      // .map(x => { console.log(x); return x; })
      .map(x => parseInt(x, 10));
  } catch (err) {
    throw processError(err);
  }
}

export async function getEmu({onSTDOUT, onSTDERR}) {
  const cpp = await getCPP();

  const ram = new cpp.MIPS$$RAM();
  const bus = new cpp.MIPS$$BUS(ram);
  const cpu = new cpp.MIPS$$CPU(bus);

  // just until I implement a native debug object, i'll reuse my old one
  const debug = new cpp.MIPS$$Debugger(cpu, ram, bus);

  cpp.print    = onSTDOUT || (s => console.log(s));
  cpp.printErr = onSTDERR || (s => console.error(s));

  return { ram, bus, cpu, debug };
}
