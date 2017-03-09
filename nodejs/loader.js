// this code is bad, and I should feel bad.

const fs = require("fs");

const cpp = require("./mips241.js");

const ram = new cpp.MIPS$$RAM();
const bus = new cpp.MIPS$$BUS(ram);
const cpu = new cpp.MIPS$$CPU(bus);

const debug = new cpp.MIPS$$Debugger(cpu, ram, bus);

const disasm = cpp.MIPS$$disasm;

function assemble(str) {
  // I didn't want to figure out why I couldn't call my jerryrigged assebler
  // function twice without reinitializing, so, uh, this is my ghetto solution.
  // pls change path on ur machine
  // Ah well ¯\_(ツ)_/¯
  delete require.cache["/Users/xtdbp/Github/mips241/nodejs/mips241.js"];
  const cpp = require("./mips241.js")

  try {
    return cpp.MIPS$$asm(str)
      .split(" ")
      .filter((x,i,a) => i !== a.length - 1)
      .map(x => { console.log(x); return x; })
      .map(x => parseInt(x, 10))
  } catch (err) {
    console.log(err)
    process.exit()
  }
}

/* Load in a `.asm` file to compile */

const prog = assemble(fs.readFileSync(process.argv[2]).toString());
prog.forEach((x, i) => { ram.store(i * 4, x) });

/* Alternatively, assemble a string */

// const prog = assemble(`
//   add $3, $1, $2
//   jr $31
// `);
// prog.forEach((x, i) => { ram.store(i * 4, x) });

/* Alternatively, load in a `.mips` file */

// const file = require('fs').readFileSync(process.argv[2]);
// for (var i = 0; i < file.length; i+=4) {
//   ram.store(i, file.readUInt32BE(i));
// }


cpu.setRegister(1, -123 >>> 0);
cpu.setRegister(2, 5 >>> 0);

console.log(`
  Initial RAM
  -----------
`);
debug.printRAMFrom(0, 30)

while (cpu.stillExecuting()) {
  for (var i = 0; i < 5; i++) {
    cpu.do_cycle();
  }

  // for (var i = 0; i < 100; i++) process.stdout.write('\n')

  // debug.addhiglight(cpu.getRegister(30))
  // debug.printRAMFrom(cpu.getRegister(30) - 10 * 4, 12);
  // debug.removehiglight(cpu.getRegister(30))

  // debug.addhiglight(cpu.getiRegister("PC"))
  // debug.printRAMFrom(Math.max(cpu.getiRegister("PC") - 4 * 6, 0), 12);
  // debug.removehiglight(cpu.getiRegister("PC"))
  // debug.printCPUState();

  process.stdout.write(bus.getOutput());
}

console.log(`
  Stack RAM
  ---------
`);
debug.addhiglight(cpu.getRegister(30))
debug.printRAMFrom(cpu.getRegister(30) - 10 * 4, 12);
debug.removehiglight(cpu.getRegister(30))

console.log(`
  Program RAM
  -----------
`);
debug.addhiglight(cpu.getiRegister("PC"))
debug.printRAMFrom(Math.max(cpu.getiRegister("PC") - 4 * 6, 0), 12);
debug.removehiglight(cpu.getiRegister("PC"))
debug.printCPUState();

ram.delete(); // nontrivial destructor

process.exit(0);

