import getCPP from './emscripten_wrapper';

import _ from 'lodash';

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

function callFunc(addr) {
  return `
    ; store return addr on stack
    sw      $31, -4($30)
    lis     $31
    .word   4
    sub     $30, $30, $31

    ; call print
    lis     $31
    .word   ${addr}
    jalr    $31

    ; restore return addr
    lis     $31
    .word   4
    add     $30, $30, $31
    lw      $31, -4($30)
  `;
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

const PRINTADDR = 0xFFFF8000;

const macros = {
  print: code => {
    return code.replace(
      /print! (\$\d+)/g,
      (match, p1, offset) => {
        if (p1 === "$1") {
          return `
            add $1, $0, ${p1}
            ${callFunc(PRINTADDR)}
          `;
        }

        // Otherwise, we need to save $1 on the stack first
        return `
          ; store $1 register on stack
          sw      $1, -4($30)
          lis     $1
          .word   4
          sub     $30, $30, $1

          add     $1, $0, ${p1}
          ${callFunc(PRINTADDR)}

          ; restore $1 register
          lis     $1
          .word   4
          add     $30, $30, $1
          lw      $1, -4($30)
        `;
      }
    );
  }
};

/*======================================
=            Exported Funcs            =
======================================*/

function preprocess(code) {
  return _.reduce(macros, (newCode, f, name) => {
    console.log(`applying the "${name}!" macro`);
    return f(newCode);
  }, code);
}

export async function assemble(code: string) : Array<number> {
  const cpp = await getCPP();
  try {
    return cpp.MIPS$$asm(preprocess(code))
      .split(" ")
      .filter((x, i, a) => i !== a.length - 1) // drop empty last element
      // .map(x => { console.log(x); return x; })
      .map(x => parseInt(x, 10));
  } catch (err) {
    throw processError(err, code);
  }
}

const getCPUProxy = cpu => {
  return new Proxy({
    // General Purpose Registers
    ..._.reduce(_.range(0, 32), (a, v) => ({ ...a, [v]: 0 }), {}),
    // Special Registers
    RA: 0, RB: 0, RZ: 0, RM: 0, RY: 0, IR: 0, PC: 0, hi: 0, lo: 0,
  }, {
    get: (target, reg) => {
      if (!_.has(target, reg))
        throw `Cannot Get Register ${reg}!`;

      if (isNaN(_.toNumber(reg))) {
        target[reg] = cpu.getiRegister(reg);
      } else {
        target[reg] = cpu.getRegister(_.toNumber(reg));
      }

      return target[reg];
    },
    set: (target, reg, val) => {
      if (!_.has(target, reg))
        throw `Cannot Set Register ${reg}!`;

      if (isNaN(_.toNumber(reg))) {
        cpu.setiRegister(reg, _.toNumber(val));
        target[reg] = val;
      } else {
        cpu.setRegister(_.toNumber(val), _.toNumber(val));
        target[reg] = val;
      }

      return true;
    }
  });
};

const getRAMProxy = ram => {
  return new Proxy({}, {
    get: (t, addr) => {
      if (addr % 4 !== 0)
        throw `Cannot Peek Memaddr 0x${addr.toString(16)}, as it isn't word aligned!`;

      return ram.load(_.toNumber(addr));
    },
    set: (t, addr, val) => {
      if (addr % 4 !== 0)
        throw `Cannot Poke Memaddr 0x${addr.toString(16)}, as it isn't word aligned!`;
      ram.store(_.toNumber(addr), _.toNumber(val));
      return true;
    }
  });
};

export async function getEmu({ onSTDOUT, onSTDERR }) {
  const cpp = await getCPP();

  const ram = new cpp.MIPS$$RAM();
  const bus = new cpp.MIPS$$BUS(ram);
  const cpu = new cpp.MIPS$$CPU(bus);

  // just until I implement a native debug object, i'll reuse my old one
  const debug = new cpp.MIPS$$Debugger(cpu, ram, bus);

  cpp.print    = onSTDOUT || (s => console.log(s));
  cpp.printErr = onSTDERR || (s => console.error(s));

  return {
    ram, bus, cpu, debug,
    proxy: {
      cpu: getCPUProxy(cpu),
      ram: getRAMProxy(ram),
    }
  };
}
