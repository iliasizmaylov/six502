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
    STATUS = FLAG_UNUSED;
    PC = PC_ADDR_RESET;
}

CPU_six502::~CPU_six502()
{
    /* No logic required yet */
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

__always_inline __flatten result_t CPU_six502::push_stack(databus_t data)
{
    result_t res = write(STKP_PAGE_RESET + STKP, data);
    STKP -= 1;
    return res;
}

__always_inline __flatten result_t CPU_six502::pop_stack(databus_t *out_data)
{
    STKP += 1;
    result_t res = read(STKP_PAGE_RESET + STKP, out_data);
    return res;
}

__always_inline __flatten result_t CPU_six502::push_pc()
{
    push_stack(HI8(PC));
    push_stack(LO8(PC));
    return SIX502_RET_SUCCESS;
}

__always_inline __flatten result_t CPU_six502::pop_pc()
{
   u8 hi;
   u8 lo;

   pop_stack(&lo);
   pop_stack(&hi);
   PC = MERGE16(hi, lo);

   return SIX502_RET_SUCCESS;
}

__always_inline __flatten void CPU_six502::set_flag(u8 flag, bool cond)
{
    if (cond)
        STATUS |= flag;
    else
        STATUS &= ~flag;
}

__always_inline __flatten u8 CPU_six502::get_flag(u8 flag)
{
    return STATUS & flag;
}

__always_inline __flatten void CPU_six502::set_flags_nz(u16 tgt)
{
    set_flag(FLAG_ZERO, (tgt & 0x00FF) == 0);
    set_flag(FLAG_NEG, !!(tgt & 0x0080));
}

__always_inline __flatten void CPU_six502::set_flags_nz(u8 tgt)
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
    if (!(STATUS & FLAG_IRQ))
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

    A = 0;
    X = 0;
    Y = 0;
    STKP = STKP_ADDR_RESET;
    STATUS = 0x00 | FLAG_UNUSED;

    ictx.abs = 0x0000;
    ictx.rel = 0x0000;

    busy_ticks = RESET_TICKS;

    return SIX502_RET_SUCCESS;
}

__always_inline bool CPU_six502::isnop()
{
    return busy_ticks <= 0;
}

result_t CPU_six502::tick()
{
    if (busy_ticks == 0) {
        memset(&ictx, 0, sizeof(ictx));

        read(PC, &ictx.opcode);
        ictx.ins = &ops[ictx.opcode];
        busy_ticks = ictx.ins->ticks;

        (this->*ictx.ins->addr)();
        (this->*ictx.ins->proc)();

        set_flag(FLAG_UNUSED, true);
    }

    busy_ticks -= busy_ticks == 0 ? 0 : 1;
    return SIX502_RET_SUCCESS;
}
