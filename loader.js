const fs = require("fs");

const cpp = require("./mips241.js");

const ram = new cpp.MIPS$$RAM();
const bus = new cpp.MIPS$$BUS(ram);
const cpu = new cpp.MIPS$$CPU(bus);

const debug = new cpp.MIPS$$Debugger(cpu, ram, bus);

const file = fs.readFileSync(process.argv[2]);

for (var i = 0; i < file.length; i+=4) {
  ram.store(i, file.readUInt32BE(i));
}

cpu.setRegister(1, 3);
cpu.setRegister(2, 5);

// debug.printRAMFrom(0x0, 30);

while(cpu.stillExecuting())
  cpu.do_cycle();

debug.printCPUState();

ram.delete(); // nontrivial destructor
