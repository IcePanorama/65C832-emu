# 65c832 Emu

## TODO
+ [ ] Create operation.ini file that links mnemoics to an operation
    + [ ] Add "flags affected" datafield to those operations.

## Opcode `code` built-ins meaning:
+ M = immediate mask operand
+ Ms = memory addressed by stack pointer (S)
+ Mx = memory addressed/indexed by X

Codes with a semicolon are processed in their same left to right order as written.

Describe as "a right-to-left evaluated postfix notation".

## Random development notes

+ the BRK (and possibly COP too) are listed as having a "base no. bytes" of 2,
but they don't actually read an operand. They just advance the program counter
by two.

## References

+ [Datasheet](https://downloads.reactivemicro.com/Electronics/CPU/W65C832%20CPU%20Datasheet%20v2.0.pdf)

