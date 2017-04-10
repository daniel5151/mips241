import getCPP from './emuwrapper';

/*=================================
=            Utilities            =
=================================*/

// dat fp doe
function getLineNumberFrom(code) {
  const lines = code.split("\n").map((ln, ln_no) => ({ln, ln_no}));
  return function (searchStr) {
    const results = lines.filter(({ ln }) => ln.search(searchStr.replace(/\$/g, "\\$")) !== -1);
    return (results.length !== 0)
      ? results[0].ln_no
      : -1;
  };
}

function processError(err, code) {
  console.log(err);

  const lineNumberOf = getLineNumberFrom(code);

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

  let m;

  // first of all, if the errors are ones I wrote, then they would have the line
  // numbers of the problematic statements. Life is easy.
  m = err.match(/line (\d+)/);
  if (m && m[1]) {
    return {
      row: parseInt(m[1], 10) - 1,
      column: 0,
      type: 'error',
      text: err.replace(/ERROR: on line \d+: /, "")
    };
  }

  // Otherwise, do some megic to get the line numbers of these other errors
  // (thrown by kind.cc and lexer.cc)
  m = err.match(/constant out of range: (\$\d+)/);
  if (m && m[1]) {
    return {
      row: lineNumberOf(m[1]),
      column: 0,
      type: 'error',
      text: `Invalid register ${m[1]}`
    };
  }

  m = err.match(/constant out of range: (-?(?:0x\w+|\d+))/);
  if (m && m[1]) {
    return {
      row: lineNumberOf(m[1]),
      column: 0,
      type: 'error',
      text: `${m[1]} is out of range!`
    };
  }

  m = err.match(/ERROR in lexing after reading (.*)/);
  if (m && m[1]) {
    return {
      row: lineNumberOf(m[1]),
      column: 0,
      type: 'error',
      text: `Lexing Error`
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

/*======================================
=            Exported Funcs            =
======================================*/

export async function assemble(code: string) : Array<number> {
  const cpp = await getCPP();
  try {
    return cpp.MIPS$$asm(code)
      .split(" ")
      .filter((x, i, a) => i !== a.length - 1) // drop empty last element
      // .map(x => { console.log(x); return x; })
      .map(x => parseInt(x, 10));
  } catch (err) {
    throw processError(err, code);
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
