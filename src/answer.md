CSC3050 Project 2 Report: Design of a MIPS Simulator

1. Environment
Programming Language: C standard: C99
Operating System: Manjaro Linux 21

2. Basic Logic
(1) Init registers, espacially PC, GP, SP and FP. 
(2) Allocate 6M for memory. 
(3) Implementing file reading to load binary and data into memory. 
(4) Setting up checkpoint. 
(5) Redirect stdin and stdout to dedicated files.
(6) Excuting the instruction PC pointing at, and update PC, looking for checkpoints. 
(7) Implementing micro instructions and syscall of mips. 

3. Impelemtation details
(1) Registers
```C
uint32_t registers[32] = {0};
uint32_t pc = 0x400000;
void init_registers()
{
    // init $gp
    registers[28] = 0x508000;
    // init sp to 0xa00000
    registers[29] = 0xa00000;
    registers[30] = 0xa00000;
}
```
(2) Memory
```C
uint8_t *memory = NULL;
memory = mmap(0, 0x600000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
```

(3) Instructions
Instructions are divided into R type, I type and J type. 
```C
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
```

Some important instructions
```C
// used to signed extending the value
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

// address translation
uint32_t memory_translate(uint32_t addr)
{
    return addr - 0x400000;
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
```

(4) System calls
```C
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
```

(5) Main execute loop
```C
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
```

4. Testing

Setting up tests directory in the folder. 

Paste the following command in the terminal. And there is no output of cmp, meaning we pass the tests. 
```Bash
make
cd tests/memcpy-hello-world
../../simulator memcpy-hello-world.asm memcpy-hello-world.txt memcpy-hello-world_checkpts.txt memcpy-hello-world.in memcpy-hello-world.out
cmp memory_55.bin correct_dump/memory_55.bin
cmp register_55.bin correct_dump/register_55.bin
cmp memcpy-hello-world.out memcpy-hello-world_correct.out
```
