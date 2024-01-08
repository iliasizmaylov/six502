#include "six502_cpu.h"
#include "six502_bus.h"

CPU_six502::CPU_six502()
{
    bus = nullptr;
    busy_ticks = 0;

    init_ops();

    memset(&ictx, 0, sizeof(ictx));

    A = 0;
    X = 0;
    Y = 0;
    STKP = STKP_ADDR_RESET;
    STATUS = 0;
    PC = PC_ADDR_RESET;

    aux_state = CPU_STATE_CLEAR;

    has_saved_state = false;
    total_ticks = 0;
}

CPU_six502::~CPU_six502()
{
    /* No logic required yet */
}

const struct instruction *CPU_six502::get_op_description(u8 opcode)
{
    return &ops[opcode];
}

void CPU_six502::save_state()
{
    memcpy(&saved_state.ictx, &ictx, sizeof(ictx));
    saved_state.A = A;
    saved_state.X = X;
    saved_state.Y = Y;
    saved_state.STKP = STKP;
    saved_state.STATUS = STATUS;
    saved_state.PC = PC;
    saved_state.busy_ticks = busy_ticks;

    has_saved_state = true;
}

void CPU_six502::load_state()
{
    if (!has_saved_state)
        return;

    memcpy(&ictx, &saved_state.ictx, sizeof(ictx));
    A = saved_state.A;
    X = saved_state.X;
    Y = saved_state.Y;
    STKP = saved_state.STKP;
    STATUS = saved_state.STATUS;
    PC = saved_state.PC;
    busy_ticks = saved_state.busy_ticks;

    has_saved_state = false;
}

result_t CPU_six502::read(addr_t addr, databus_t *out_data)
{
    if (!bus)
        return SIX502_RET_NO_BUS;

    return bus->broadcast_read(addr, out_data);
}

result_t CPU_six502::write(addr_t addr, databus_t data)
{
    if (!bus)
        return SIX502_RET_NO_BUS;

    return bus->broadcast_write(addr, data);
}

result_t CPU_six502::push_stack(databus_t data)
{
    result_t res = write(STKP_PAGE_RESET | STKP, data);
    STKP -= 1;
    return res;
}

result_t CPU_six502::pop_stack(databus_t *out_data)
{
    STKP += 1;
    result_t res = read(STKP_PAGE_RESET | STKP, out_data);
    return res;
}

result_t CPU_six502::push_pc()
{
    push_stack(HI8(PC));
    push_stack(LO8(PC));
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::pop_pc()
{
   u8 hi;
   u8 lo;

   pop_stack(&lo);
   pop_stack(&hi);
   PC = MERGE16(hi, lo);

   return SIX502_RET_SUCCESS;
}

void CPU_six502::set_flag(u8 flag, bool cond)
{
    if (cond)
        STATUS |= flag;
    else
        STATUS &= ~flag;
}

u8 CPU_six502::get_flag(u8 flag)
{
    return STATUS & flag;
}

void CPU_six502::set_flags_nz(u16 tgt)
{
    set_flag(FLAG_ZERO, (tgt & 0x00FF) == 0);
    set_flag(FLAG_NEG, !!(tgt & 0x0080));
}

void CPU_six502::set_flags_nz(u8 tgt)
{
    set_flags_nz((u16)tgt);
}

result_t CPU_six502::bind(BUS_six502 *new_bus)
{
    if (!new_bus)
        return SIX502_RET_BAD_INPUT;

    bus = new_bus;
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::irq()
{
    if (STATUS & FLAG_IRQ)
        return SIX502_RET_IRQ_DISABLED;

    push_pc();

    STATUS |= FLAG_UNUSED | FLAG_IRQ;
    STATUS &= ~FLAG_BREAK;

    push_stack(STATUS);

    ictx.abs = IRQ_ADDR_RESET;

    u8 lo;
    read(ictx.abs, &lo);

    u8 hi;
    read(ictx.abs + 1, &hi);

    PC = MERGE16(hi, lo);

    busy_ticks = IRQ_TICKS;

    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::nmi()
{
    push_pc();

    STATUS &= ~FLAG_BREAK;
    STATUS |= FLAG_UNUSED | FLAG_IRQ;

    push_stack(STATUS);

    ictx.abs = NMI_ADDR_RESET;

    u8 lo;
    read(ictx.abs, &lo);

    u8 hi;
    read(ictx.abs + 1, &hi);

    PC = MERGE16(hi, lo);

    busy_ticks = NMI_TICKS;

    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::reset()
{
    ictx.abs = PC_ADDR_RESET;

    u8 lo;
    read(ictx.abs, &lo);

    u8 hi;
    read(ictx.abs + 1, &hi);

    PC = MERGE16(hi, lo);
    ictx.opaddr = PC;
    read(PC, &ictx.opcode);
    ictx.ins = &ops[ictx.opcode];

    A = 0;
    X = 0;
    Y = 0;
    STKP = STKP_ADDR_RESET;
    STATUS = 0x00 | FLAG_UNUSED;

    ictx.abs = 0x0000;
    ictx.rel = 0x0000;

    busy_ticks = RESET_TICKS;

    aux_state = CPU_STATE_CLEAR;
    total_ticks = 0;

    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::__runop(bool debugger_on)
{
    memset(&ictx, 0, sizeof(ictx));

    ictx.opaddr = PC;
    read(PC++, &ictx.opcode);
    ictx.ins = &ops[ictx.opcode];

    if (!debugger_on)
        busy_ticks = ictx.ins->ticks;
    total_ticks += busy_ticks;

    (this->*ictx.ins->addr)();

    (this->*ictx.ins->proc)();

    set_flag(FLAG_UNUSED, true);

    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::dryrun()
{
    struct instruction_ctx saved_ctx;

    this->save_state();

    memset(&ictx, 0, sizeof(ictx));

    ictx.opaddr = PC;
    read(PC++, &ictx.opcode);
    ictx.ins = &ops[ictx.opcode];

    busy_ticks = ictx.ins->ticks;

    (this->*ictx.ins->addr)();

    memcpy(&saved_ctx, &ictx, sizeof(ictx));
    this->load_state();
    memcpy(&ictx, &saved_ctx, sizeof(ictx));

    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::runop()
{
    return __runop(false);
}

result_t CPU_six502::runop_dbg()
{
    return __runop(true);
}

result_t CPU_six502::tick()
{
    if (busy_ticks == 0)
        this->runop();

    total_ticks += 1;
    busy_ticks -= busy_ticks == 0 ? 0 : 1;
    return SIX502_RET_SUCCESS;
}
