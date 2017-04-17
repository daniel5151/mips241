// utlity defn
declare type uint32_t = number;


// Ideally, if I were to spend some more time with this project, I should
// implement a custom debugger. But alas, this project is starting to drag on
// too long for my liking, so i'm just leaving it with a bare minimum
// implementation
declare type MIPS$$Debugger = {
  addhiglight(addr: uint32_t): void;
  printRAMFrom(addr: uint32_t, lookahead: number): void;
  removehiglight(addr: uint32_t): void;
  printCPUState(): void;

  new(cpu: MIPS$$CPU, ram: MIPS$$RAM, bus: MIPS$$BUS): MIPS$$Debugger;
};

// The meat of the emulator
declare type MIPS$$asm = (code: string) => string;

declare type MIPS$$RAM = {
  load(addr: uint32_t): uint32_t;
  store(addr: uint32_t, val: uint32_t): void;

  delete(): void;
  new(): MIPS$$RAM;
};
declare type MIPS$$BUS = {
  load(addr: uint32_t): uint32_t;
  store(addr: uint32_t, val: uint32_t): void;

  getOutput(): string;

  new(ram: MIPS$$RAM): MIPS$$BUS;
};
declare type MIPS$$CPU = {
  stillExecuting(): boolean;
  do_cycle(): void;

  getCycle(): number;
  getStage(): number;

  getPC(): uint32_t;

  getRegister(reg: number): uint32_t;
  setRegister(reg: number, val: uint32_t): void;

  getiRegister(reg: string): uint32_t;
  setiRegister(reg: string, val: uint32_t): void;

  new(bus: MIPS$$BUS): MIPS$$CPU;
};

declare type Mips241 = {
  MIPS$$asm: MIPS$$asm;
  MIPS$$RAM: MIPS$$RAM;
  MIPS$$BUS: MIPS$$BUS;
  MIPS$$CPU: MIPS$$CPU;
  MIPS$$Debugger: MIPS$$Debugger;

  print: (s: string) => void;
  printErr: (s: string) => void;
};
