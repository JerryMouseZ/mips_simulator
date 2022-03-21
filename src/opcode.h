#ifndef OPCODE_H
#define OPCODE_H

// for Funct
#define ADD     0x20
#define ADDU    0x21
#define AND     0x24
#define DIV     0x1a
#define DIVU    0x1b
#define JALR    0x09
#define JR      0x08
#define MFHI    0x10
#define MFLO    0x12
#define MTHI    0x11
#define MTLO    0x13
#define MULT    0x18
#define MULTU   0x19
#define NOR     0x27
#define OR      0x25
#define SLL     0x00
#define SLLV    0x04
#define SLT     0x2a
#define SLTU    0x2b
#define SRA     0x03
#define SRAV    0x07
#define SRL     0x02
#define SRLV    0x06
#define SUB     0x22
#define SUBU    0x23
#define SYSCALL 0x0c
#define XOR     0x26

// for opcode of I
#define ADDI    0x08
#define ADDIU   0x09
#define ANDI    0x0c
#define BEQ     0x04
#define BGEZ    0x01
#define BGTZ    0x07
#define BLEZ    0x06
#define BLTZ    0x01
#define BNE     0x05
#define LB      0x20
#define LBU     0x24
#define LH      0x21
#define LHU     0x25
#define LUI     0x0f
#define LW      0x23
#define ORI     0x0d
#define SB      0x28
#define SLTI    0x0a
#define SLTIU   0x0b
#define SH      0x29
#define SW      0x2b
#define XORI    0x0e
#define LWL     0x22
#define LWR     0x26
#define SWL     0x2a
#define SWR     0x2e

// for opcode of J
#define J       0x2
#define JAL     0x03

#endif
