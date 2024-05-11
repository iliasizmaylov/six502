#include "six502_cpu.h"

__six502_instr result_t CPU_six502::iSLO()
{
    write(ictx.abs, ictx.imm);
    set_flag(FLAG_CARRY, !!((ictx.imm >> 7) & 1));
    ictx.imm <<= 1;
    A |= ictx.imm;
    write(ictx.abs, ictx.imm);
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iRLA()
{
    write(ictx.abs, ictx.imm);
    ictx.imm = (ictx.imm << 1) | (u8)get_flag(FLAG_CARRY);
    set_flag(FLAG_CARRY, !!((ictx.imm >> 7) & 1));
    write(ictx.abs, ictx.imm);
    A &= ictx.imm;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSRE()
{
    write(ictx.abs, ictx.imm);
    set_flag(FLAG_CARRY, !!(ictx.imm & 1));
    ictx.imm >>= 1;
    write(ictx.abs, ictx.imm);
    A ^= ictx.imm;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iRRA()
{
    write(ictx.abs, ictx.imm);
    ictx.imm = (ictx.imm >> 1) | ((u8)get_flag(FLAG_CARRY) << 7);
    set_flag(FLAG_CARRY, !!(ictx.imm & 1));
    write(ictx.abs, ictx.imm);
    ictx.aux = A + ictx.imm + (u8)get_flag(FLAG_CARRY);
    set_flag(FLAG_CARRY, (ictx.aux & 0xFF00) > 0);
    set_flag(FLAG_OFLOW, (((A ^ ictx.aux) & (ictx.imm ^ ictx.aux)) & 0x0080) > 0);
    A = (u8)ictx.aux;

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSAX()
{
    return write(ictx.abs, A & X);
}

__six502_instr result_t CPU_six502::iLAX()
{
    read(ictx.abs, &A);
    X = A;
    set_flags_nz(A);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iDCP()
{
    ictx.imm--;
    write(ictx.abs, ictx.imm);
    ictx.aux = A - ictx.imm;
    set_flag(FLAG_CARRY, A >= ictx.imm);
    set_flags_nz((u8)ictx.aux);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iISC()
{
    ictx.imm++;
    write(ictx.abs, ictx.imm);
    ictx.aux = A - ictx.imm - (u8)get_flag(FLAG_CARRY);
    set_flag(FLAG_CARRY, (ictx.aux & 0xFF00) == 0);
    set_flag(FLAG_OFLOW, !!(((A ^ ictx.imm) & (A ^ ictx.aux)) & 0x0080));
    A = (u8)ictx.aux;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iANC()
{
    A &= ictx.imm;
    set_flags_nz(A);
    set_flag(FLAG_CARRY, get_flag(FLAG_NEG));

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iALR()
{
    A &= ictx.imm;
    set_flag(FLAG_CARRY, !!(A & 1));
    A >>= 1;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iARR()
{
    A &= ictx.imm;
    A = (A >> 1) | ((u8)get_flag(FLAG_CARRY) << 7);
    set_flags_nz(A);
    set_flag(FLAG_CARRY, !!((A >> 6) & 1));
    set_flag(FLAG_OFLOW, !!(((A >> 5) & 1) ^ (u8)get_flag(FLAG_CARRY)));

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iXAA()
{
    A = (A | 0xEE) & X & ictx.imm;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iAXS()
{
    ictx.aux = (X & A) - ictx.imm;
    X = (u8)ictx.aux;
    set_flags_nz(X);
    set_flag(FLAG_CARRY, (X & A) >= ictx.imm);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iAHX()
{
    ictx.aux = (A & X) & 7;
    write(ictx.abs, (u8)ictx.aux);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSHY()
{
    ictx.aux = Y & ((ictx.abs >> 8) + 1);
    read(PC + 1, &ictx.aux81);
    ictx.aux2 = Y + ictx.aux81;

	write(ictx.abs, (u8)ictx.aux);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSHX()
{
    ictx.aux = X & ((ictx.abs >> 8) + 1);
    read(PC + 1, &ictx.aux81);
    ictx.aux2 = X + ictx.aux81;

	write(ictx.abs, (u8)ictx.aux);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iTAS()
{
    STKP = A & X;
    ictx.aux = STKP & (ictx.abs >> 8);
    write(ictx.abs, (u8)ictx.abs);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iLAS()
{
    ictx.aux = ictx.imm & STKP;
    A = (u8)ictx.aux;
    X = (u8)ictx.aux;
    STKP = (u8)ictx.aux;

    return SIX502_RET_SUCCESS;
}
