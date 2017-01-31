const fs = require("fs");

const cpp = require("./mips241.js");

const ram = new cpp.MIPS$$RAM();
const bus = new cpp.MIPS$$BUS(ram);
const cpu = new cpp.MIPS$$CPU(bus);

const debug = new cpp.MIPS$$Debugger(cpu, ram, bus);

const disasm = cpp.MIPS$$disasm;

const file = require('fs').readFileSync(process.argv[2]);

for (var i = 0; i < file.length; i+=4) {
  ram.store(i, file.readUInt32BE(i));
}

cpu.setRegister(1, 128);
cpu.setRegister(2, 5);

debug.printRAMFrom(0, 70)

var readline = require('readline');
var rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  terminal: false
});

rl.on('line', function(line){
  if (cpu.stillExecuting() === false) process.exit(0);

  for (var i = 0; i < 5; i++) {
    cpu.do_cycle();
  }

  process.stdout.write(bus.getOutput())


  // for (var i = 0; i < 100; i++) process.stdout.write('\n')

  // debug.addhiglight(cpu.getRegister(30))
  // debug.printRAMFrom(cpu.getRegister(30) - 10 * 4, 12);
  // debug.removehiglight(cpu.getRegister(30))

  // debug.addhiglight(cpu.getiRegister("PC"))
  // debug.printRAMFrom(Math.max(cpu.getiRegister("PC") - 4 * 6, 0), 12);
  // debug.removehiglight(cpu.getiRegister("PC"))
  // debug.printCPUState();
})

  // process.stdout.write(bus.getOutput())


// ram.delete(); // nontrivial destructor
