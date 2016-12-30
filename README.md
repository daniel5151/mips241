# mips241
An emulator for the MIPS system used in CS 241 @ uWaterloo

No guarantees that it's perfect and/or bug-free, but hey, at least it's free ¯\\\_(ツ)\_/¯

Aside from running faster than `java mips.twoints` / `java mips.array` (looking at you *java*), it also has a plethora of useful debugging features! 

Starting the emulator with the `--debug` flag throws you into a debug interface
that looks a little something like this:

```
  ----------====== Stack RAM ======---------
     ADDR    |   HEXVAL   :     MIPS ASM    
  -----------|------------------------------
  0x00fffff0 | 0x00000000 : .word 0x00000000
  0x00fffff4 | 0x00000000 : .word 0x00000000
  0x00fffff8 | 0x00000000 : .word 0x00000000
  0x00fffffc | 0x00000000 : .word 0x00000000
> 0x01000000 | 0x00000000 : .word 0x00000000
  0x01000004 | 0x00000000 : .word 0x00000000
  0x01000008 | 0x00000000 : .word 0x00000000
  0x0100000c | 0x00000000 : .word 0x00000000
  0x01000010 | 0x00000000 : .word 0x00000000
  0x01000014 | 0x00000000 : .word 0x00000000
  0x01000018 | 0x00000000 : .word 0x00000000
  0x0100001c | 0x00000000 : .word 0x00000000

  ---------====== Program RAM ======--------
     ADDR    |   HEXVAL   :     MIPS ASM    
  -----------|------------------------------
  0x00000000 | 0x00002014 : lis   $4
  0x00000004 | 0x00000004 : .word 0x00000004
> 0x00000008 | 0x00005814 : lis   $11
  0x0000000c | 0x00000001 : .word 0x00000001
  0x00000010 | 0x00003014 : lis   $6
  0x00000014 | 0xffff0004 : .word 0xffff0004
  0x00000018 | 0x00003814 : lis   $7
  0x0000001c | 0xffff000c : .word 0xffff000c
  0x00000020 | 0x8cc30000 : lw    $3, 0($6)
  0x00000024 | 0xace30000 : sw    $3, 0($7)
  0x00000028 | 0x002b0822 : sub   $1, $1, $11
  0x0000002c | 0x1401fffc : bne   $0, $1, -4
  0x00000030 | 0x03e00008 : jr    $31
  0x00000034 | 0x00000000 : .word 0x00000000
  0x00000038 | 0x00000000 : .word 0x00000000
  0x0000003c | 0x00000000 : .word 0x00000000

-------------------------====== CPU State ======-------------------------
$1  = 0x00000001   $2  = 0x00000002   $3  = 0x00000000   $4  = 0x00000004
$5  = 0x00000000   $6  = 0x00000000   $7  = 0x00000000   $8  = 0x00000000
$9  = 0x00000000   $10 = 0x00000000   $11 = 0x00000000   $12 = 0x00000000
$13 = 0x00000000   $14 = 0x00000000   $15 = 0x00000000   $16 = 0x00000000
$17 = 0x00000000   $18 = 0x00000000   $19 = 0x00000000   $20 = 0x00000000
$21 = 0x00000000   $22 = 0x00000000   $23 = 0x00000000   $24 = 0x00000000
$25 = 0x00000000   $26 = 0x00000000   $27 = 0x00000000   $28 = 0x00000000
$29 = 0x00000000   $30 = 0x01000000   $31 = 0x8123456c
 PC = 0x00000008    IR = 0x00002014

 lo = 0x00000000    hi = 0x00000000

 RA = 0x00000000    RB = 0x00000000
 RZ = 0x00000004
 RM = 0x00000000
 RY = 0x00000004
Stage: 0
Cycle no. 5
step> 

```

From here, you can enter any of these many *fantastic* debug commands:

```
step              - exec one instruction
run               - resume regular execution
+bp <addr>        - add breakpoint at <addr>
-bp <addr>        - remove breakpoint at <addr>
peek <addr>       - print out contents of <addr>
poke <addr> <val> - edit contents of <addr>
+watch <addr>     - keep an eye on <addr>
-watch <addr>     - don't keep an eye on <addr>
print             - display debug data
```

# \_disasm

Also included is a standalone disassembler. Give it your `.mips` or `.merl`
files, and it'll spit out a `.asm` file.

# Trivia

`mips241` uses the same processor datapath as taught in ECE 222. 

Why? 

Well, I wanted to make it so that at some point in the future I could have taken
a crack at pipelining this emulated processor. I never did, but if any of you
Software Engineers out there want to take a shot at doing that, go ahead!