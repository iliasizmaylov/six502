#include "six502_cpu.h"

static __always_inline u8 __branch_on(bool cond, u16 *pc, const u16 rel)
{
    u8 pagex = (u8)(cond & (HI8(*pc + rel) != HI8(*pc)));
    *pc += cond ? rel : 0;
    return pagex;
}

__six502_instr result_t CPU_six502::iBPL()
{
    busy_ticks += __branch_on(!get_flag(FLAG_NEG), &PC, ictx.rel);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iBMI()
{
    busy_ticks += __branch_on(get_flag(FLAG_NEG), &PC, ictx.rel);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iBVC()
{
    busy_ticks += __branch_on(!get_flag(FLAG_OFLOW), &PC, ictx.rel);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iBVS()
{
    busy_ticks += __branch_on(get_flag(FLAG_OFLOW), &PC, ictx.rel);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iBCC()
{
    busy_ticks += __branch_on(!get_flag(FLAG_CARRY), &PC, ictx.rel);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iBCS()
{
    busy_ticks += __branch_on(get_flag(FLAG_CARRY), &PC, ictx.rel);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iBNE()
{
    busy_ticks += __branch_on(!get_flag(FLAG_ZERO), &PC, ictx.rel);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iBEQ()
{
    busy_ticks += __branch_on(get_flag(FLAG_ZERO), &PC, ictx.rel);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iKIL()
{
    return SIX502_RET_HALT;
}