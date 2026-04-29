Syntax (standard assembly pattern):
[label:] [instruction/directive] [operands] [; comment]

Registers: R0-R7
Numbers:
    hex 0x0ABF
    dec 1234
    bin 0b01001010

Directives:
    .org ADDRESS 
    todo: more

ISA:

Memory map:
0x0000 - 0x000F | VECTORS   | 0x0000: RESET Vector
0x0010 - 0x03FF | RESERVED  | System use / I/O mapping
0x0400 - 0x7FFF | TEXT      | Program 
0x8000 - 0xDFFF | DATA      | Heap
0xE000 - 0xFFFF | STACK     | Grows down (but kinda up?) from 0xFFFF

Instruction format:
type A (16-bit): [ 8-bit OPCODE ] [ 4-bit reg_a ] [ 4-bit reg_b ]
type B (32-bit): [ 8-bit OPCODE ] [ 4-bit reg_a ] [ 4-bit reg_b ] + [ 16-bit IMM ]

reg_a is the upper 4 bits of the second byte; reg_b is the lower 4 bits.
for instructions taking only one register, it goes into reg_a (reg_b = 0).
for instructions taking no registers (HALT, RET, JMP, CALL), both fields = 0.


System:
0x00 HALT             (16-bit)  Stops execution.
0x11 OUT   R          (16-bit)  Prints value of R.
                                reg_a = SRC

Data:
0x01 SET   Rd, IMM    (32-bit)  Rd = IMM.
                                reg_a = DST
0x02 MOV   Rd, Rs     (16-bit)  Rd = Rs.
                                reg_a = DST, reg_b = SRC
0x03 LOAD  Rd, [Rs]   (16-bit)  Rd = Memory[Rs].
                                reg_a = DST, reg_b = ADDR
0x04 STORE Rs, [Rd]   (16-bit)  Memory[Rd] = Rs.
                                reg_a = ADDR, reg_b = SRC
                                  (note: assembly syntax order is inverted relative to encoding)

Logic and maths:
0x05 ADD   Rd, Rs     (16-bit)  Rd = Rd + Rs.
                                reg_a = DST + SRC, reg_b = SRC
0x06 SUB   Rd, Rs     (16-bit)  Rd = Rd - Rs.
                                reg_a = DST + SRC, reg_b = SRC
0x07 AND   Rd, Rs     (16-bit)  Rd = Rd & Rs.
                                reg_a = DST + SRC, reg_b = SRC
0x08 OR    Rd, Rs     (16-bit)  Rd = Rd | Rs.
                                reg_a = DST + SRC, reg_b = SRC
0x09 LSH   Rd, IMM    (32-bit)  Rd = Rd << IMM.
                                reg_a = DST + SRC

Control flow:
0x0A JMP   ADDR       (32-bit)  PC = IMM.
0x0B JE    Rd, Rs, A  (32-bit)  If Rd == Rs, PC = IMM.
                                reg_a = SRC, reg_b = SRC
0x0C JG    Rd, Rs, A  (32-bit)  If Rd > Rs,  PC = IMM.
                                reg_a = SRC, reg_b = SRC

0x0D PUSH  R          (16-bit)  SP -= 2; Mem[SP] = R.
                                reg_a = SRC
0x0E POP   R          (16-bit)  R = Mem[SP]; SP += 2.
                                reg_a = DST
0x0F CALL  ADDR       (32-bit)  Push(PC); PC = IMM.
0x10 RET              (16-bit)  PC = Pop()
