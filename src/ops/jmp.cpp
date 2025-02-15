#include "six502_cpu.h"

namespace SIX502 {

static inline u8 __branch_on(bool cond, u16 *pc,
        const u16 rel, CPU_six502 *cpu)
{
    addr_t pc_before = *pc;
    u8 pagex = (u8)(cond & (HI8((*pc + rel) != HI8(*pc))));
    *pc += cond ? rel : 0;

    if (cond && *pc == (pc_before - 2))
        cpu->aux_state |= CPU_JUMP_SELF;

    return pagex;
}

result_t CPU_six502::iBPL()
{
    busy_ticks += __branch_on(!get_flag(FLAG_NEG), &PC, ictx.rel, this);
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::iBMI()
{
    busy_ticks += __branch_on(get_flag(FLAG_NEG), &PC, ictx.rel, this);
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::iBVC()
{
    busy_ticks += __branch_on(!get_flag(FLAG_OFLOW), &PC, ictx.rel, this);
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::iBVS()
{
    busy_ticks += __branch_on(get_flag(FLAG_OFLOW), &PC, ictx.rel, this);
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::iBCC()
{
    busy_ticks += __branch_on(!get_flag(FLAG_CARRY), &PC, ictx.rel, this);
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::iBCS()
{
    busy_ticks += __branch_on(get_flag(FLAG_CARRY), &PC, ictx.rel, this);
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::iBNE()
{
    busy_ticks += __branch_on(!get_flag(FLAG_ZERO), &PC, ictx.rel, this);
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::iBEQ()
{
    busy_ticks += __branch_on(get_flag(FLAG_ZERO), &PC, ictx.rel, this);
    return SIX502_RET_SUCCESS;
}

result_t CPU_six502::iKIL()
{
    return SIX502_RET_HALT;
}

} /* namespace SIX502 */
