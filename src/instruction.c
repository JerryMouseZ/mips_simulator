#include "instruction.h"
#include "parser.h"
#include "opcode.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

extern uint32_t pc;
extern uint8_t *memory;
extern uint32_t registers[32];
uint32_t rhi = 0;
uint32_t rlo = 0;
uint32_t program_break = 0;
int ins_count = 0;

/* #ifndef DEBUG */
/* #define fprintf(...) (void)(0) */
/* #endif */

static uint32_t SIGNEX(uint32_t val, uint32_t offset)
{
    if (offset == 8) {
        return (int32_t)(int8_t) val;
    }
    if (offset == 16) {
        return (int32_t)(int16_t) val;
    }
    uint32_t mask = 1;
    mask <<= (offset - 1);
    if (val & mask) {
        for (int i = offset; i < 32; ++i) {
            mask <<= 1;
            val += mask;
        }
    }
    return val;
}


/*
 * 0x400000 start of text
 * 0x500000 end of text, start of static data segment
 * data segment end, and dynamic data begin
 * 0xa00000 the begin of the stack
 */
uint32_t memory_translate(uint32_t addr)
{
    return addr - 0x400000;
}

void mips_add(uint32_t rd, uint32_t rs, uint32_t rt)
{
    registers[rd] = registers[rs] + registers[rt];
}

void mips_addu(uint32_t rd, uint32_t rs, uint32_t rt)
{
    registers[rd] = registers[rs] + registers[rt];
}

void mips_addi(uint32_t rt, uint32_t rs, uint32_t imm)
{
    uint32_t rel = SIGNEX(imm, 16);
    registers[rt] = registers[rs] + rel;
}

void mips_addiu(uint32_t rt, uint32_t rs, uint32_t imm)
{
    registers[rt] = registers[rs] + SIGNEX(imm, 16);
}


void mips_sub(uint32_t rd, uint32_t rs, uint32_t rt)
{
    registers[rd] = registers[rs] - registers[rt];
}

void mips_subu(uint32_t rd, uint32_t rs, uint32_t rt)
{
    registers[rd] = registers[rs] - registers[rt];
}

void mips_mult(uint32_t rs, uint32_t rt)
{
    uint64_t res = registers[rs];
    res *= registers[rt];
    rhi = res >> 32;
    rlo = res;
}

void mips_multu(uint32_t rs, uint32_t rt)
{
    mips_mult(rs, rt);
}

void mips_div(uint32_t rs, uint32_t rt)
{
    rlo = registers[rs] / registers[rt];
    rhi = registers[rs] % registers[rt];
}

void mips_divu(uint32_t rs, uint32_t rt)
{
    mips_div(rs, rt);
}

void mips_mflo(uint32_t rd)
{
    registers[rd] = rlo;
}

void mips_mfhi(uint32_t rd)
{
    registers[rd] = rhi;
}

void mips_mtlo(uint32_t rs)
{
    rlo = registers[rs];
}

void mips_mthi(uint32_t rs)
{
    rhi = registers[rs];
}

void mips_and(uint32_t rd, uint32_t rs, uint32_t rt)
{
    registers[rd] = registers[rs] & registers[rt];
}

void mips_andi(uint32_t rt, uint32_t rs, uint32_t imm)
{
    registers[rt] = registers[rs] & imm;
}

void mips_or(uint32_t rd, uint32_t rs, uint32_t rt)
{
    registers[rd] = registers[rs] | registers[rt];
}

void mips_ori(uint32_t rt, uint32_t rs,  uint32_t imm)
{
    registers[rt] = registers[rs] | imm;
}

void mips_nor(uint32_t rd, uint32_t rs, uint32_t rt)
{
    registers[rd] = ~(registers[rs] | registers[rt]);
}

void mips_xor(uint32_t rd, uint32_t rs, uint32_t rt)
{
    registers[rd] = registers[rs] ^ registers[rt];
}

void mips_xori(uint32_t rt, uint32_t rs,  uint32_t imm)
{
    registers[rt] = registers[rs] ^ imm;
}

void mips_sll(uint32_t rd, uint32_t rt, uint32_t sa)
{
    registers[rd] = registers[rt] << sa;
}

void mips_sllv(uint32_t rd, uint32_t rt, uint32_t rs)
{
    registers[rd] = registers[rt] << registers[rs];
}

void mips_srl(uint32_t rd, uint32_t rt, uint32_t sa)
{
    // append 0 in the left
    registers[rd] = registers[rt] >> sa;
}

void mips_srlv(uint32_t rd, uint32_t rt, uint32_t rs)
{
    // append 0 in the left
    registers[rd] = registers[rt] >> registers[rs];
}

void mips_sra(uint32_t rd, uint32_t rt, uint32_t sa)
{
    // append 1 in the left
    registers[rd] = (int)registers[rt] >> sa;
}

void mips_srav(uint32_t rd, uint32_t rt, uint32_t rs)
{
    // append 1 in the left
    registers[rd] = (int)registers[rt] >> registers[rs];
}

void mips_slt(uint32_t rd, uint32_t rs, uint32_t rt)
{
    registers[rd] = registers[rs] < registers[rt];
}

void mips_sltu(uint32_t rd, uint32_t rs, uint32_t rt)
{
    mips_slt(rd, rs, rt);
}

void mips_slti(uint32_t rt, uint32_t rs, uint32_t imm)
{
    registers[rt] = (int)registers[rs] < SIGNEX(imm, 16);
}

void mips_sltiu(uint32_t rt, uint32_t rs, uint32_t imm)
{
    mips_slti(rt, rs, imm);
}

void mips_beq(uint32_t rs, uint32_t rt, uint32_t offset)
{
    int disp = SIGNEX(offset, 16);
    if (registers[rs] == registers[rt]) {
        pc += disp << 2;
    }
}

void mips_bne(uint32_t rs, uint32_t rt, uint32_t offset)
{
    int disp = SIGNEX(offset, 16);
    if (registers[rs] != registers[rt]) {
        pc += disp << 2;
    }
}

void mips_bgtz(uint32_t rs, uint32_t offset)
{
    int disp = SIGNEX(offset, 16);
    if ((int)(registers[rs]) > 0) {
        pc += disp << 2;
    }
}

void mips_blez(uint32_t rs, uint32_t offset)
{
    int disp = SIGNEX(offset, 16);
    if ((int)(registers[rs]) <= 0) {
        pc += disp << 2;
    }
}

void mips_bgez(uint32_t rs, uint32_t offset)
{
    int disp = SIGNEX(offset, 16);
    if (registers[rs] >= 0) {
        pc += disp << 2;
    }
}

void mips_bltz(uint32_t rs, uint32_t offset)
{
    int disp = SIGNEX(offset, 16);
    if ((int)registers[rs] < 0) {
        pc += disp << 2;
    }
}

void mips_j(uint32_t offset)
{
    int disp = offset;
    pc = disp << 2;
}

void mips_jal(uint32_t offset)
{
    registers[31] = pc;
    int disp = offset;
    pc = disp << 2;
}

void mips_jalr(uint32_t rs)
{
    registers[31] = pc;
    pc = registers[rs];
}

void mips_jr(uint32_t rs)
{
    pc = registers[rs];
}

void mips_lb(uint32_t rt, uint32_t rs, uint32_t imm)
{
    registers[rt] = memory[memory_translate(registers[rs] + imm)];
    registers[rt] = SIGNEX(registers[rt], 8);
}

void mips_lbu(uint32_t rt, uint32_t rs,  uint32_t imm)
{
    registers[rt] = memory[memory_translate(registers[rs] + imm)];
}

void mips_lh(uint32_t rt,  uint32_t rs, uint32_t imm)
{
    registers[rt] = *(uint16_t *)(memory + memory_translate(registers[rs] + imm));
    registers[rt] = SIGNEX(registers[rt], 16);
}

void mips_lhu(uint32_t rt, uint32_t rs,  uint32_t imm)
{
    registers[rt] = *(uint16_t *)(memory + memory_translate(registers[rs] + imm));
}

void mips_lui(uint32_t rt, uint32_t imm)
{
    registers[rt] = imm << 16;
}

void mips_lw(uint32_t rt, uint32_t rs,  uint32_t imm)
{

    registers[rt] = *(uint32_t *)(memory + memory_translate(registers[rs] + imm));
}

void mips_sb(uint32_t rt, uint32_t rs, uint32_t imm)
{
    memory[memory_translate(registers[rs] + imm)] = registers[rt];
}

void mips_sh(uint32_t rt, uint32_t rs, uint32_t imm)
{
    *(uint16_t *)(memory + memory_translate(registers[rs] + imm)) = registers[rt];
}

void mips_sw(uint32_t rt, uint32_t rs, uint32_t imm)
{
    *(uint32_t *)(memory + memory_translate(registers[rs] + imm)) = registers[rt];
}

void mips_lwl(uint32_t rt, uint32_t rs,  uint32_t imm)
{
    uint32_t addr = registers[rs] + imm;
    uint8_t *res = (uint8_t *)&registers[rt];
    addr = memory_translate(addr);
    for (uint32_t i = addr; i < (addr + 3) / 4 * 4; ++i) {
        res[i - addr] = memory[i];
    }
}

void mips_lwr(uint32_t rt, uint32_t rs,  uint32_t imm)
{
    uint32_t addr = registers[rs] + imm;
    uint8_t *res = (uint8_t *)&registers[rt];
    addr = memory_translate(addr);
    int begin = addr / 4 * 4;
    int count = addr - begin + 1;
    for (int i = 4 - count; i < 4; ++i) {
        res[i] = memory[i + addr + count - 4];
    }
}

void mips_swl(uint32_t rt, uint32_t rs,  uint32_t imm)
{
    uint32_t addr = registers[rs] + imm;
    uint8_t *res = (uint8_t *)&registers[rt];
    addr = memory_translate(addr);
    for (uint32_t i = addr; i < (addr + 3) / 4 * 4; ++i) {
        memory[i] = res[i - addr];
    }
}

void mips_swr(uint32_t rt, uint32_t rs,  uint32_t imm)
{
    uint32_t addr = registers[rs] + imm;
    uint8_t *res = (uint8_t *)&registers[rt];
    addr = memory_translate(addr);
    int begin = addr / 4 * 4;
    int count = addr - begin + 1;
    for (int i = 4 - count; i < 4; ++i) {
        memory[i + addr + count - 4] = res[i];
    }
}

void mips_syscall()
{
    switch(registers[2])
    {
        case 1:
            // print int
            printf("%d", registers[4]);
            break;
        case 4:
            // print string
            printf("%s", memory + memory_translate(registers[4]));
            break;
        case 5:
            {
                // read int
                char buffer[12];
                fgets(buffer, 12, stdin);
                registers[2] = atoi(buffer);
                /* scanf("%d\n", &registers[2]); */
                break;
            }
        case 8:
            {
                // read string
                char *buffer = (char *)memory + memory_translate(registers[4]);
                fgets(buffer, registers[5], stdin);
                buffer[strlen(buffer) - 1] = 0;
                break;
            }
        case 9:
            // sbrk
            registers[2] = program_break;
            program_break += registers[4];
            break;
        case 10:
            // exit
            exit(0);
            break;
        case 11:
            // print char
            printf("%c", (char)registers[4]);
            break;
        case 12:
            // read char
            registers[2] = 0;
            scanf("%c\n", (char *)&registers[2]);
            break;
        case 13:
            // open file
            registers[4] = open((char *)memory + memory_translate(registers[4]), registers[5], registers[6]);
            break;
        case 14:
            // read
            registers[4] = read(registers[4], memory + memory_translate(registers[5]), registers[6]);
            break;
        case 15:
            // write
            registers[4] = write(registers[4], memory + memory_translate(registers[5]), registers[6]);
            break;
        case 16:
            close(registers[4]);
            break;
        case 17:
            exit(registers[4]);
            break;
        default:
            fprintf(stderr, "unsupported syscall code %d\n", registers[2]);
            exit(-1);
    }
}

static const uint32_t funct_mask = 0b111111;
static const uint32_t reg_mask = 0b11111;

// R-Format instructions
void exec_r(uint32_t inst)
{
    uint32_t funct = inst & funct_mask;
    uint32_t sa = (inst >> 6) & reg_mask;
    uint32_t rd = (inst >> 11) & reg_mask;
    uint32_t rt = (inst >> 16) & reg_mask;
    uint32_t rs = (inst >> 21) & reg_mask;
    switch (funct) {
    case ADD:
        fprintf(stderr, "%d: add\n", ins_count);
        mips_add(rd, rs, rt);
        break;
    case ADDU:
        fprintf(stderr, "%d: addu\n", ins_count);
        mips_addu(rd, rs, rt);
        break;
    case AND:
        fprintf(stderr, "%d: and\n", ins_count);
        mips_and(rd, rs, rt);
        break;
    case DIV:
        fprintf(stderr, "%d: div\n", ins_count);
        mips_div(rd, rs);
        break;
    case DIVU:
        fprintf(stderr, "%d: divu\n", ins_count);
        mips_divu(rs, rt);
        break;
    case JALR:
        fprintf(stderr, "%d: jalr\n", ins_count);
        mips_jalr(rs);
        break;
    case JR:
        fprintf(stderr, "%d: jr\n", ins_count);
        mips_jr(rs);
        break;
    case MFHI:
        fprintf(stderr, "%d: mfhi\n", ins_count);
        mips_mfhi(rd);
        break;
    case MFLO:
        fprintf(stderr, "%d: mflo\n", ins_count);
        mips_mflo(rd);
        break;
    case MTHI:
        fprintf(stderr, "%d: mfhi\n", ins_count);
        mips_mthi(rs);
        break;
    case MTLO:
        fprintf(stderr, "%d: mflo\n", ins_count);
        mips_mtlo(rs);
        break;
    case MULT:
        fprintf(stderr, "%d: mult\n", ins_count);
        mips_mult(rs, rt);
        break;
    case MULTU:
        fprintf(stderr, "%d: multu\n", ins_count);
        mips_multu(rs, rt);
        break;
    case NOR:
        fprintf(stderr, "%d: nor\n", ins_count);
        mips_nor(rd, rs, rt);
        break;
    case OR:
        fprintf(stderr, "%d: or\n", ins_count);
        mips_or(rd, rs, rt);
        break;
    case SLL:
        fprintf(stderr, "%d: sll\n", ins_count);
        mips_sll(rd, rt, sa);
        break;
    case SLLV:
        fprintf(stderr, "%d: sllv\n", ins_count);
        mips_sllv(rd, rt, rs);
        break;
    case SLT:
        fprintf(stderr, "%d: slt\n", ins_count);
        mips_slt(rd, rs, rt);
        break;
    case SLTU:
        fprintf(stderr, "%d: slt\n", ins_count);
        mips_sltu(rd, rs, rt);
        break;
    case SRA:
        fprintf(stderr, "%d: sra\n", ins_count);
        mips_sra(rd, rt, sa);
        break;
    case SRAV:
        fprintf(stderr, "%d: srav\n", ins_count);
        mips_srav(rd, rt, rs);
        break;
    case SRL:
        fprintf(stderr, "%d: srl\n", ins_count);
        mips_srl(rd, rt, sa);
    case SRLV:
        fprintf(stderr, "%d: srlv\n", ins_count);
        mips_srlv(rd, rt, rs);
        break;
    case SUB:
        fprintf(stderr, "%d: sub\n", ins_count);
        mips_sub(rd, rs, rt);
        break;
    case SUBU:
        fprintf(stderr, "%d: subu\n", ins_count);
        mips_subu(rd, rs, rt);
        break;
    case XOR:
        fprintf(stderr, "%d: xor\n", ins_count);
        mips_xor(rd, rs, rt);
        break;
    case SYSCALL:
        fprintf(stderr, "%d: syscall %d\n", ins_count, registers[2]);
        mips_syscall();
        break;
    }
}

void exec_i(uint32_t inst)
{
    uint32_t opcode = inst >> 26;
    uint32_t imm = inst & 0xffff;
    uint32_t rt = (inst >> 16) & reg_mask;
    uint32_t rs = (inst >> 21) & reg_mask;
    switch (opcode) {
    case ADDI:
        fprintf(stderr, "%d: addi\n", ins_count);
        mips_addi(rt, rs, imm);
        break;
    case ADDIU:
        fprintf(stderr, "%d: addiu\n", ins_count);
        mips_addiu(rt, rs, imm);
        break;
    case ANDI:
        fprintf(stderr, "%d: andi\n", ins_count);
        mips_andi(rt, rs, imm);
        break;
    case BEQ:
        fprintf(stderr, "%d: beq\n", ins_count);
        mips_beq(rs, rt, imm);
        break;
    case BGEZ:
        // for bgez and bltz
        if (rt == 1) {
            fprintf(stderr, "%d: bgez\n", ins_count);
            mips_bgez(rs, imm);
        } else if (rt == 0) {
            fprintf(stderr, "%d: bltz\n", ins_count);
            mips_bltz(rs, imm);
        } else {
            assert(0);
        }
        break;
    case BGTZ:
        if (rt == 0) {
            fprintf(stderr, "%d: bgtz\n", ins_count);
            mips_bgtz(rs, imm);
        } else {
            assert(0);
        }
        break;
    case BLEZ:
        if (rt == 0) {
            fprintf(stderr, "%d: blez\n", ins_count);
            mips_blez(rs, imm);
        } else {
            assert(0);
        }
        break;
    case BNE:
        fprintf(stderr, "%d: bne\n", ins_count);
        mips_bne(rs, rt, imm);
        break;
    case LB:
        fprintf(stderr, "%d: lb\n", ins_count);
        mips_lb(rt, rs, imm);
        break;
    case LBU:
        fprintf(stderr, "%d: lbu\n", ins_count);
        mips_lbu(rt, rs, imm);
        break;
    case LH:
        fprintf(stderr, "%d: lh\n", ins_count);
        mips_lh(rt, rs, imm);
        break;
    case LHU:
        fprintf(stderr, "%d: lhu\n", ins_count);
        mips_lhu(rt, rs, imm);
        break;
    case LUI:
        fprintf(stderr, "%d: lui\n", ins_count);
        mips_lui(rt, imm);
        break;
    case LW:
        fprintf(stderr, "%d: lw\n", ins_count);
        mips_lw(rt, rs, imm);
        break;
    case ORI:
        fprintf(stderr, "%d: ori\n", ins_count);
        mips_ori(rt, rs, imm);
        break;
    case SB:
        fprintf(stderr, "%d: sb\n", ins_count);
        mips_sb(rt, rs, imm);
        break;
    case SLTI:
        fprintf(stderr, "%d: slti\n", ins_count);
        mips_slti(rt, rs, imm);
        break;
    case SLTIU:
        fprintf(stderr, "%d: sltiu\n", ins_count);
        mips_sltiu(rt, rs, imm);
        break;
    case SH:
        fprintf(stderr, "%d: sh\n", ins_count);
        mips_sh(rt, rs, imm);
        break;
    case SW:
        fprintf(stderr, "%d: sw\n", ins_count);
        mips_sw(rt, rs, imm);
        break;
    case XORI:
        fprintf(stderr, "%d: xori\n", ins_count);
        mips_xori(rt, rs, imm);
        break;
    case LWL:
        fprintf(stderr, "%d: lwl\n", ins_count);
        mips_lwl(rt, rs, imm);
        break;
    case LWR:
        fprintf(stderr, "%d: lwr\n", ins_count);
        mips_lwr(rt, rs, imm);
        break;
    case SWL:
        fprintf(stderr, "%d: swl\n", ins_count);
        mips_swl(rt, rs, imm);
        break;
    case SWR:
        fprintf(stderr, "%d: swr\n", ins_count);
        mips_swr(rt, rs, imm);
        break;
    default:
        fprintf(stderr, "invalid instruction %d\n", opcode);
        exit(-1);
    }
}

void exec_j(uint32_t inst)
{
    uint32_t opcode = inst >> 26;
    uint32_t target = (inst << 6) >> 6;
    if (opcode == J)
    {
        fprintf(stderr, "%d: j\n", ins_count);
        mips_j(target);
    } else if (opcode == JAL) {
        fprintf(stderr, "%d: jal\n", ins_count);
        mips_jal(target);
    }
}

void execute()
{
    while (pc < 0x500000) {
        // checkpoint
        checkpoint_register(ins_count);
        checkpoint_memory(ins_count);
        ins_count++;

        // fetch instruction
        uint32_t inst = *(uint32_t *) (memory + memory_translate(pc));

        // update pc
        pc += 4;

        // execute
        uint32_t opcode = inst >> 26;
        if (opcode == 0) {
            exec_r(inst);
        } else if (opcode == J || opcode == JAL) {
            exec_j(inst);
        } else {
            exec_i(inst);
        }
    }
}
