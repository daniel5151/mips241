# mips241 - emscripten'd

## What is this?

This is an experimental branch where I'm playing around with emscripten, and
seeing if it is possible to port `mips241` to the web.

## Hey, why isn't this compiling?!

Well, the `_asm` directory is not actually included in this repo!

Writing a mips assembler is an actual assignment in CS 241, and as such, me
putting up my assembler would (probably) be a violation of accademic policy.

That said, you can always just plop in your own assembler.

Just make sure it has the following signature:

`std::string assemble(std::string filecontent)`

where `filecontent` is the `.asm` file as a string, and the function returns a
space seperated string of unsigned ints (in base 10) to load into memory.

Eg:

```
lis   $19
.word 0xffff000c
lis   $20
.word 0x00000004
```

becomes `198688 32798754 2917269504 337706998`

_You may have to edit the_ `Makefile` _to get it to build properly_.
