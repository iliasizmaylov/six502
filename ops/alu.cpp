#include "six502_cpu.h"

__six502_instr result_t CPU_six502::iORA()
{
    A |= ictx.imm;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iAND()
{
    A &= ictx.imm;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iEOR()
{
    A ^= ictx.imm;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

#define OVERFLOW_FLAG_ADC(reg, imm, res)   \
    (!!((~((u16)reg ^ (u16)imm) & ((u16)reg ^ (u16)res)) & 0x0080))

__six502_instr result_t CPU_six502::iADC()
{
    ictx.aux = (u16)A +
            (u16)ictx.imm +
            (u16)get_flag(FLAG_CARRY);

    set_flag(FLAG_CARRY, ictx.aux > 0x00FF);
    set_flag(FLAG_OFLOW, OVERFLOW_FLAG_ADC(A, ictx.imm, ictx.aux));

    set_flags_nz(ictx.aux);

    A = (u8)(ictx.aux & 0x00FF);

    return SIX502_RET_SUCCESS;
}

#undef OVERFLOW_FLAG_ADC

#define OVERFLOW_FLAG_SBC(reg, imm, res)   \
    (!!((((u16)reg ^ (u16)imm) & ((u16)res ^ (u16)imm)) & 0x0080))

__six502_instr result_t CPU_six502::iSBC()
{
    ictx.aux = ((u16)ictx.imm) ^ 0x00FF;
    ictx.aux2 = (u16)A +
            ictx.aux +
            (u16)get_flag(FLAG_CARRY);

    set_flag(FLAG_CARRY, !!(ictx.aux2 & 0xFF00));
    set_flag(FLAG_OFLOW, OVERFLOW_FLAG_SBC(A, ictx.aux2, ictx.aux));
    set_flags_nz(ictx.aux2);

    A = ictx.aux2 & 0x00FF;

    return SIX502_RET_SUCCESS;
}

#undef OVERFLOW_FLAG_SBC

__six502_instr result_t CPU_six502::iCMP()
{
    ictx.aux = (u16)A - (u16)ictx.imm;
    set_flag(FLAG_CARRY, A >= ictx.imm);
    set_flags_nz(ictx.aux);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iCPX()
{
    ictx.aux = (u16)X - (u16)ictx.imm;
    set_flag(FLAG_CARRY, X >= ictx.imm);
    set_flags_nz(ictx.aux);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iCPY()
{
    ictx.aux = (u16)Y - (u16)ictx.imm;
    set_flag(FLAG_CARRY, Y >= ictx.imm);
    set_flags_nz(ictx.aux);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iDEC()
{
    ictx.aux = ictx.imm - 1;
    write(ictx.abs, ictx.aux & 0x00FF);
    set_flags_nz(ictx.aux);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iDEX()
{
    X--;
    set_flags_nz(X);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iDEY()
{
    Y--;
    set_flags_nz(Y);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iINC()
{
    ictx.aux = ictx.imm + 1;
    write(ictx.abs, ictx.aux & 0x00FF);
    set_flags_nz(ictx.aux);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iINX()
{
    X++;
    set_flags_nz(X);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iINY()
{
    Y++;
    set_flags_nz(Y);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iASL()
{
    ictx.aux = (u16)ictx.imm << 1;

    set_flags_nz(ictx.aux);
    set_flag(FLAG_CARRY, (ictx.aux & 0xFF00) > 0);

    if (ictx.ins->addr == &CPU_six502::aIMP)
        A = ictx.aux & 0x00FF;
    else
        write(ictx.abs, ictx.aux & 0x00FF);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iROL()
{
    ictx.aux = (u16)(ictx.imm << 1) | get_flag(FLAG_CARRY);

    set_flags_nz(ictx.aux);
    set_flag(FLAG_CARRY, (ictx.aux & 0xFF00) > 0);

    if (ictx.ins->addr == &CPU_six502::aIMP)
        A = ictx.aux & 0x00FF;
    else
        write(ictx.abs, ictx.aux & 0x00FF);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iLSR()
{
    ictx.aux = (u16)ictx.imm >> 1;

    set_flags_nz(ictx.aux);
    set_flag(FLAG_CARRY, !!(ictx.imm & 0x0001));

    if (ictx.ins->addr == &CPU_six502::aIMP)
        A = ictx.aux & 0x00FF;
    else
        write(ictx.abs, ictx.aux & 0x00FF);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iROR()
{
    ictx.aux = (u16)(get_flag(FLAG_CARRY) << 7) | (u16)ictx.imm >> 1;

    set_flags_nz(ictx.aux);
    set_flag(FLAG_CARRY, !!(ictx.imm & 0x0001));

    if (ictx.ins->addr == &CPU_six502::aIMP)
        A = ictx.aux & 0x00FF;
    else
        write(ictx.abs, ictx.aux & 0x00FF);

    return SIX502_RET_SUCCESS;
}
