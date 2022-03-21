#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>

// R instruction
void mips_add(uint32_t rd, uint32_t rs, uint32_t rt);
void mips_addu(uint32_t rd, uint32_t rs, uint32_t rt);
void mips_and(uint32_t rd, uint32_t rs, uint32_t rt);
void mips_div(uint32_t rs, uint32_t rt);
void mips_divu(uint32_t rs, uint32_t rt);
void mips_jalr(uint32_t rs);
void mips_jr(uint32_t rs);
void mips_mfhi(uint32_t rd);
void mips_mflo(uint32_t rd);
void mips_mthi(uint32_t rs);
void mips_mtlo(uint32_t rs);
void mips_mult(uint32_t rs, uint32_t rt);
void mips_multu(uint32_t rs, uint32_t rt);
void mips_nor(uint32_t rd, uint32_t rs, uint32_t rt);
void mips_or(uint32_t rd, uint32_t rs, uint32_t rt);
void mips_sll(uint32_t rd, uint32_t rt, uint32_t sa);
void mips_sllv(uint32_t rd, uint32_t rt, uint32_t rs);
void mips_slt(uint32_t rd, uint32_t rs, uint32_t rt);
void mips_sltu(uint32_t rd, uint32_t rs, uint32_t rt);
void mips_sra(uint32_t rd, uint32_t rt, uint32_t sa);
void mips_srav(uint32_t rd, uint32_t rt, uint32_t rs);
void mips_srl(uint32_t rd, uint32_t rt, uint32_t sa);
void mips_srlv(uint32_t rd, uint32_t rt, uint32_t rs);
void mips_sub(uint32_t rd, uint32_t rs, uint32_t rt);
void mips_subu(uint32_t rd, uint32_t rs, uint32_t rt);
void mips_syscall();
void mips_xor(uint32_t rd, uint32_t rs, uint32_t rt);

// I instruction
void mips_addi(uint32_t rt, uint32_t rs, uint32_t imm);
void mips_addiu(uint32_t rt, uint32_t rs, uint32_t imm);
void mips_andi(uint32_t rt, uint32_t rs, uint32_t imm);
void mips_beq(uint32_t rs, uint32_t rt, uint32_t offset);
void mips_beqz(uint32_t rs, uint32_t offset);
void mips_bgtz(uint32_t rs, uint32_t offset);
void mips_blez(uint32_t rs, uint32_t offset);
void mips_bltz(uint32_t rs, uint32_t offset);
void mips_bne(uint32_t rs, uint32_t rt, uint32_t offset);
void mips_lb(uint32_t rt, uint32_t rs, uint32_t imm);
void mips_lbu(uint32_t rt, uint32_t rs,  uint32_t imm);
void mips_lh(uint32_t rt,  uint32_t rs, uint32_t imm);
void mips_lhu(uint32_t rt, uint32_t rs,  uint32_t imm);
void mips_lui(uint32_t rt, uint32_t imm);
void mips_lw(uint32_t rt, uint32_t rs,  uint32_t imm);
void mips_ori(uint32_t rt, uint32_t rs,  uint32_t imm);
void mips_sb(uint32_t rt, uint32_t rs, uint32_t imm);
void mips_sh(uint32_t rt, uint32_t rs, uint32_t imm);
void mips_sw(uint32_t rt, uint32_t rs, uint32_t imm);
void mips_slti(uint32_t rt, uint32_t rs, uint32_t imm);
void mips_sltiu(uint32_t rt, uint32_t rs, uint32_t imm);
void mips_xori(uint32_t rt, uint32_t rs,  uint32_t imm);
void mips_lwl(uint32_t rt, uint32_t rs,  uint32_t imm);
void mips_lwr(uint32_t rt, uint32_t rs,  uint32_t imm);
void mips_swl(uint32_t rt, uint32_t rs,  uint32_t imm);
void mips_swr(uint32_t rt, uint32_t rs,  uint32_t imm);

// J instruction
void mips_j(uint32_t offset);
void mips_jal(uint32_t offset);

void execute();

#endif
