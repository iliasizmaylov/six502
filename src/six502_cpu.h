#ifndef _SIX502_CPU_H_
#define _SIX502_CPU_H_

#include "six502_dev.h"

#include <iostream>
#include <unordered_map>

/* Flags for status register */
enum __STATUS_REG_FLAGS {
    FLAG_CARRY  = (1 << 0),     /* Carry flag */
    FLAG_ZERO   = (1 << 1),     /* Zero flag */
    FLAG_IRQ    = (1 << 2),     /* IRQ flag */
    FLAG_DEC    = (1 << 3),     /* Decimal flag */
    FLAG_BREAK  = (1 << 4),     /* Break flag */
    FLAG_UNUSED = (1 << 5),     /* This flag is unused by 6502 */
    FLAG_OFLOW  = (1 << 6),     /* Overflow flag */
    FLAG_NEG    = (1 << 7)      /* Negative flag */
};

/* Various built-in addresses and values for irq, reset, nmi, etc.*/
enum __CPU_BUILTIN_STATS {
    PC_ADDR_RESET   = 0xFFFC,   /* Reset vector address */
    STKP_ADDR_RESET = 0x00FF,   /* Max stack offset from stack page 0x0100 */
    STKP_PAGE_RESET = 0x0100,   /* Stack page in RAM */
    IRQ_ADDR_RESET  = 0xFFFE,   /* IRQ vector address */
    NMI_ADDR_RESET  = 0xFFFA,   /* NMI vector address */
    IRQ_TICKS       = 7,        /* NR of ticks consumed by an interrupt */
    NMI_TICKS       = 8,        /* NR of ticks consumed by NMI */
    RESET_TICKS     = 8         /* NR of ticks consumed by reset */
};

/* Custom CPU state that currenly only indicates
 * three different conditions (apart from clear
 * state which is an absence of any extra conditions)
 */
enum __CPU_AUX_STATES {
    CPU_STATE_CLEAR     = 0,

    /* Non-error condition that indicates if stack pointer
     * overlapped 0x00 back to 0xFF which is a normal 6502 behavior
     */
    CPU_STACK_OVERFLOW  = (1 << 15),

    /* Also a non-error condition but maybe indicative
     * of a flaw in a program that's being run
     * Indicates if there was a load from stack when
     * stack pointer was set 0xFF
     */
    CPU_STACK_EMPTY_POP = (1 << 14),

    /* This condition is also not considered an error in
     * 6502 cpu but could be a problem in a lot of scenarios
     * like maybe a hard assert or something
     * This flag is usefulf when running a six502 debugger
     * because it will break at self jumps
     */
    CPU_JUMP_SELF       = (1 << 13)
};

class BUS_six502;
class CPU_six502;

/* Instruction descriptor struct */
struct instruction {
    u8 opcode;                  /* 1-Byte instruction code */
    std::string readable;       /* Human readable instruction name */
    std::string a_readable;     /* Human readable address mode name */
    u8 ticks;                   /* Number of ticks required by instruction */

    /* Pointer to instruction handler function */
    result_t (CPU_six502::*proc)();

    /* Pointer to address mode handler function */
    result_t (CPU_six502::*addr)();
};

/* Context for the current instruction being executed */
struct instruction_ctx {
    u8 opcode;
    addr_t opaddr;
    addr_t abs;     /* Aboslute address of an instruction target */
    addr_t rel;     /* Relative address required by current instruction */
    addr_t rel_d;   /* Relative address required by current instruction */
    u8 imm;         /* Fetched value after applying address mode handler */
    u16 aux;        /* Auxiliary container 1 */
    u16 aux2;       /* Auxiliary container 2 */
    u8 aux81;       /* Auxiliary 8-bit container 1 */
    u8 aux82;       /* Auxiliary 8-bit container 2 */
    u8 aux83;       /* Auxiliary 8-bit container 2 */

    /* Pointer to an instruction that is currently being executed */
    struct instruction *ins;
};

class CPU_six502 {
public:
    CPU_six502();
    ~CPU_six502();

private:
    BUS_six502 *bus;    /* Pointer to BUS class that this CPU is associated with */
    u8 busy_ticks;      /* Number of ticks left until CPU is ready to execute next instruction */

public:
    /* Map of: hex opcode<->instruction descriptor struct */
    std::unordered_map<u8, struct instruction> ops;

    /* Context for the current instruction being executed */
    struct instruction_ctx ictx;

    u8 get_flag(u8 flag);

    const struct instruction *get_op_description(u8 opcode);

private:
    /* Called once in constructor, initializes this->ops */
    void init_ops();

    /* Stack operations */
    result_t push_stack(databus_t data);
    result_t pop_stack(databus_t *out_data);
    result_t push_pc();
    result_t pop_pc();

    /* Status register operations */
    void set_flag(u8 flag, bool cond);
    void set_flags_nz(u16 tgt);
    void set_flags_nz(u8 tgt);

    /* Runs one instruction at PC
     * If debugger_on == true then CPU will not count
     * ticks (or cycles) that executed instruction 
     * takes to run which is done for six502 debugger
     * single step mode
     */
    result_t __runop(bool debugger_on);

public:
    /* Read from the BUS (this->bus) */
    result_t read(addr_t addr, databus_t *out_data);

    /* Write to the BUS (this->bus) */
    result_t write(addr_t addr, databus_t data);

    u8 A;       /* A register */
    u8 X;       /* X register */
    u8 Y;       /* Y register */
    u8 STKP;    /* Stack pointer */
    u8 STATUS;  /* Status register */
    addr_t PC;  /* Program counter */

    u16 aux_state;      /* Custom CPU state */
    u64 total_ticks;    /* Total 6502 ticks (cycles) since start */

    /* Bind a bus to this CPU */
    result_t bind(BUS_six502 *new_bus);

    result_t irq();     /* Interrupt request handler */
    result_t nmi();     /* Non-Maskable interrupt handler */
    result_t reset();   /* CPU reset handler */

    /* dryrun is just what it sounds like - it processes 
     * one instruction at PC, fills this->ictx but doesn't 
     * let the instructions to actually do it's business
     * and affect the emulated system 
     * It's used for debugger to gather info about instruction
     * i.e. what's the address mode, immediate value or absolute
     * address, zp offset, etc. */
    result_t dryrun();

    result_t runop();               /* Run single instruction (non-debugger mode) */
    result_t runop_dbg();           /* Run single instruction (debgger mode) */
    result_t tick();                /* Processor tick (i.e. clock tick handler) */
    result_t tick_for(u64 nticks);

    /* This struct describes a full state of a CPU so that
     * we can use data stored in here to be able to revert
     * to a given state
     *
     * Used when six502 debugger is doing a disassembly
     * 
     * Basically it's here in order for us to be able to
     * do multiple this->dryrun() in order to get a 
     * disassembly for a chunk of memory 
     *
     * TODO: It'd be much better if we disassemble full memory first
     * and not do this tedious iterative disassemble at every step*/
    struct saved_state_desc {
        struct instruction_ctx ictx;
        u8 A;
        u8 X;
        u8 Y;
        u8 STKP;
        u8 STATUS;
        addr_t PC;
        u8 busy_ticks;
    } saved_state;

    /* Flag indicating if a state has been saved */
    bool has_saved_state;

    void save_state();  /* Saves current CPU state into this->saved_state */
    void load_state();  /* Loads current CPU state from this->saved_state */

private:
    /* Address mode handlers */
    result_t aIMP();    /* Implied address mode */
    result_t aIMM();    /* Immediate address mode */
    result_t aZP0();    /* Zero page address mode */
    result_t aZPX();    /* Zero page with X offset address mode */
    result_t aZPY();    /* Zero page with Y offset address mode */
    result_t aREL();    /* Relative address mode */
    result_t aABS();    /* Absolute address mode */
    result_t aABX();    /* Absolute with X offset address mode */
    result_t aABY();    /* Absolute with Y offset address mode */
    result_t aAXR();    /* Absolute with X offset address mode */
    result_t aAYR();    /* Absolute with Y offset address mode */
    result_t aIND();    /* Indirect address mode */
    result_t aIZX();    /* Indirect with X offset address mode */
    result_t aIZY();    /* Indirect with Y offset address mode */
    result_t aIXR();    /* Indirect with Y offset address mode */
    result_t aIYR();    /* Indirect with Y offset address mode */

    /* Official instruction handlers: Logical and arithmetic */
    result_t iORA();    /* Logical OR with A register */
    result_t iAND();    /* Logical AND with A register */
    result_t iEOR();    /* Logical XOR with A register */
    result_t iADC();    /* Add to A register */
    result_t iSBC();    /* Substract from A register */
    result_t iCMP();    /* Compare with A register */
    result_t iCPX();    /* Compare with X register */
    result_t iCPY();    /* Compare with Y register */
    result_t iDEC();    /* Decrease target by 1 */
    result_t iDEX();    /* Decrease X register by 1 */
    result_t iDEY();    /* Decrease Y register by 1 */
    result_t iINC();    /* Increase target by 1 */
    result_t iINX();    /* Increase X register by 1 */
    result_t iINY();    /* Increase Y register by 1 */
    result_t iASL();    /* Arithmetic shift left */
    result_t iROL();    /* Arithmetic shift left with carry */
    result_t iLSR();    /* Arithmetic shift right */
    result_t iROR();    /* Arithmetic shift right with carry */

    /* Official instruction handlers: Movers */
    result_t iLDA();    /* Load address into A register */
    result_t iSTA();    /* Store A register at address */
    result_t iLDX();    /* Load address into X register */
    result_t iSTX();    /* Store X register at address */
    result_t iLDY();    /* Load address into Y register */
    result_t iSTY();    /* Store Y register at address */
    result_t iTAX();    /* Store A register into X register */
    result_t iTXA();    /* Store X register into A register */
    result_t iTAY();    /* Store A register into Y register */
    result_t iTYA();    /* Store Y register into A register */
    result_t iTSX();    /* Store stack pointer into X register */
    result_t iTXS();    /* Set stack pointer to the value of X register */
    result_t iPLA();    /* Pop A register from stack */
    result_t iPHA();    /* Push A register into stack */
    result_t iPLP();    /* Pop status register from stack */
    result_t iPHP();    /* Push status register into stack */

    /* Official instruction handlers: Jumps and flags */
    result_t iBPL();    /* Branch if NOT NEGATIVE */
    result_t iBMI();    /* Branch if NEGATIVE */
    result_t iBVC();    /* Branch if NOT OVERFLOW */
    result_t iBVS();    /* Branch if OVERFLOW */
    result_t iBCC();    /* Branch if NOT CARRY */
    result_t iBCS();    /* Branch if CARRY */
    result_t iBNE();    /* Branch if NOT ZERO */
    result_t iBEQ();    /* Branch if ZERO */
    result_t iBRK();    /* Break */
    result_t iRTI();    /* P,PC:=+(S) */
    result_t iJSR();    /* (S)-:=PC PC:={addr} */
    result_t iRTS();    /* PC:=+(S) */
    result_t iJMP();    /* Jump to given address */
    result_t iBIT();    /* Update status register bits */
    result_t iCLC();    /* Clear CARRY flag */
    result_t iSEC();    /* Set CARRY flag */
    result_t iCLD();    /* Clear DECIMAL flag */
    result_t iSED();    /* Set DECIMAL flag */
    result_t iCLI();    /* Clear INTERRUPT flag */
    result_t iSEI();    /* Set INTERRUPT flag */
    result_t iCLV();    /* Clear OVERFLOW flag */
    result_t iNOP();    /* No operation (does nothing) */

    /* Unofficial instruction handlers (a.k.a. Illegal opcodes) */
    result_t iSLO();    /* {addr}:={addr}*2 A:=A or {addr} */
    result_t iRLA();    /* {addr}:={addr}rol A:=A and {addr} */
    result_t iSRE();    /* {addr}:={addr}/2 A:=A xor {addr} */
    result_t iRRA();    /* {addr}:={addr}ror A:=A adc {addr} */
    result_t iSAX();    /* {addr}:=A&X */
    result_t iLAX();    /* A,X:={addr} */
    result_t iDCP();    /* {addr}:={addr}-1 A:=A-{addr} */
    result_t iISC();    /* {addr}:={addr}+1 A:=A-{addr} */
    result_t iANC();    /* A:=A&#{imm} */
    result_t iACC();    /* A:=A&#{imm} */
    result_t iALR();    /* A:=(A&#{imm})/2 */
    result_t iARR();    /* A:=(A&#{imm})/2 */
    result_t iXAA();    /* A:=X&#{imm} */
    result_t iAXS();    /* X:=A&X-#{imm} */
    result_t iAHX();    /* A:=A-#{imm} */
    result_t iSHY();    /* A:=A-#{imm} */
    result_t iSHX();    /* A:=A-#{imm} */
    result_t iTAS();    /* A:=A-#{imm} */
    result_t iLAS();    /* A,X,S:={addr}&S */
    result_t iKIL();    /* Halts the CPU, data bus will be set to 0xFF */
};

#endif  /* _SIX502_CPU_H_ */
