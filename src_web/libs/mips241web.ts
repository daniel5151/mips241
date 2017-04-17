import getCPP from './emscripten_wrapper';

import * as _ from 'lodash';

/*=================================
=            Utilities            =
=================================*/

// dat fp doe
function getLineNumberFrom(code: string) {
  const lines = code.split("\n").map((ln, lnNo) => ({ln, lnNo}));
  return function (searchStr: string) {
    const results = lines.filter(({ ln }) => ln.search(searchStr.replace(/\$/g, "\\$")) !== -1);
    return (results.length !== 0)
      ? results[0].lnNo
      : -1;
  };
}

function callFunc(addr: number) {
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

function processError(errStr: string, code: string) {
  console.log(errStr);

  // this shouldn't happen, but alas, it probably will ¯\_(ツ)_/¯
  if (typeof errStr === "number") {
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
  m = errStr.match(/line (\d+)/);
  if (m && m[1]) {
    return {
      row: parseInt(m[1], 10) - 1,
      column: 0,
      type: 'error',
      text: errStr.replace(/ERROR: on line \d+: /, "")
    };
  }

  // Otherwise, do some megic to get the line numbers of these other errors
  // (thrown by kind.cc and lexer.cc)
  const lineNumberOf = getLineNumberFrom(code);

  m = errStr.match(/constant out of range: (\$\d+)/);
  if (m && m[1]) {
    return {
      row: lineNumberOf(m[1]),
      column: 0,
      type: 'error',
      text: `Invalid register ${m[1]}`
    };
  }

  m = errStr.match(/constant out of range: (-?(?:0x\w+|\d+))/);
  if (m && m[1]) {
    return {
      row: lineNumberOf(m[1]),
      column: 0,
      type: 'error',
      text: `${m[1]} is out of range!`
    };
  }

  m = errStr.match(/ERROR in lexing after reading (.*)/);
  if (m && m[1]) {
    return {
      row: lineNumberOf(m[1]),
      column: 0,
      type: 'error',
      text: `Lexing Error`
    };
  }

  // Check for invalid opcode
  m = errStr.match(/Invalid opcode/);
  if (m && m[1]) {
    return {
      row: 0,
      column: 0,
      type: 'error',
      text: `Invalid Opcode`
    };
  }

  // If all else fails...
  return {
    row: 0,
    column: 0,
    type: 'error',
    text: errStr
  };
}

const PRINTADDR = 0xFFFF8000;

type preload = { [name: string]: { code: Uint32Array, addr: uint32_t } };
const preload: preload = {
  "print": {
    code: require('~/libs/mips_imports/print.merl'),
    addr: PRINTADDR
  }
};

type macros = { [name: string]: (code: string) => string };
const macros: macros = {
  print: (code) => {
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

export async function assemble(code: string): Promise<Array<number>> {
  const cpp = await getCPP();
  try {
    const processedCode = _.reduce(macros, (newCode, f, name) => {
      console.log(`applying the "${name}!" macro`);
      return f(newCode);
    }, code);

    const words = cpp.MIPS$$asm(processedCode).split(" ");
    words.pop();

    return words.map(x => parseInt(x, 10));
  } catch (err) {
    throw processError(err, code);
  }
}

const getCPUProxy = (cpu: MIPS$$CPU) => {
  return new Proxy({
    // General Purpose Registers
     1: 0,  2: 0,  3: 0,  4: 0,  5: 0,  6: 0,  7: 0,  8: 0,  9: 0, 10: 0,
    11: 0, 12: 0, 13: 0, 14: 0, 15: 0, 16: 0, 17: 0, 18: 0, 19: 0, 20: 0,
    21: 0, 22: 0, 23: 0, 24: 0, 25: 0, 26: 0, 27: 0, 28: 0, 29: 0, 30: 0,
    31: 0,
    // Special Registers
    RA: 0, RB: 0, RZ: 0, RM: 0, RY: 0, IR: 0, PC: 0, hi: 0, lo: 0,
  }, {
    get: (target, reg) => {
      if (!_.has(target, reg)) {
        throw `Cannot Get Register ${reg}!`;
      }

      if (isNaN(_.toNumber(reg))) {
        target[reg] = cpu.getiRegister(reg.toString());
      } else {
        target[reg] = cpu.getRegister(_.toNumber(reg));
      }

      return target[reg];
    },
    set: (target, reg, val) => {
      if (!_.has(target, reg)) {
        throw `Cannot Set Register ${reg}!`;
      }

      if (isNaN(_.toNumber(reg))) {
        cpu.setiRegister(reg.toString(), _.toNumber(val));
        target[reg] = val;
      } else {
        cpu.setRegister(_.toNumber(val), _.toNumber(val));
        target[reg] = val;
      }

      return true;
    }
  });
};

// tslint:disable-next-line:no-any
function returnOf<R> (f: (...args: any[]) => R): {returnType: R} { return null!; }

// mfw it's kludgy af to get a return type of a function in typescript
let dummy = returnOf(getCPUProxy);
export type CPUProxy = typeof dummy.returnType;

const getRAMProxy = (ram: MIPS$$RAM) => {
  return new Proxy({}, {
    get: (t, addr: number) => {
      // if (typeof addr !== "number") { throw "ehh"; }
      if (addr % 4 !== 0) {
        throw `Cannot Peek Memaddr 0x${addr.toString(16)}, as it isn't word aligned!`;
      }

      return ram.load(_.toNumber(addr));
    },
    set: (t, addr: number, val) => {
      // if (typeof addr !== "number") { throw "ehh"; }
      if (addr % 4 !== 0) {
        throw `Cannot Poke Memaddr 0x${addr.toString(16)}, as it isn't word aligned!`;
      }
      ram.store(_.toNumber(addr), _.toNumber(val));
      return true;
    }
  });
};

type outStreams = {
  onSTDOUT?: (s: string) => void,
  onSTDERR?: (s: string) => void
};

export async function getEmu({ onSTDOUT, onSTDERR }: outStreams) {
  const cpp = await getCPP();

  const ram = new cpp.MIPS$$RAM();
  const bus = new cpp.MIPS$$BUS(ram);
  const cpu = new cpp.MIPS$$CPU(bus);

  // just until I implement a native debug object, i'll reuse my old one
  const debug = new cpp.MIPS$$Debugger(cpu, ram, bus);

  cpp.print    = onSTDOUT || (s => console.log(s));
  cpp.printErr = onSTDERR || (s => console.error(s));

  _.forEach(preload, ({ code, addr }, name) => {
    console.log(`loading "${name}" code into RAM at address 0x${_.padStart(addr.toString(16), 8, '0')}`);
    _.forEach(code, (x, i) => ram.store(addr + i * 4, x));
  });

  return {
    ram, bus, cpu, debug,
    proxy: {
      cpu: getCPUProxy(cpu),
      ram: getRAMProxy(ram),
    }
  };
}
