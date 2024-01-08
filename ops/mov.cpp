#include "six502_cpu.h"

__six502_instr result_t CPU_six502::iLDA()
{
    A = ictx.imm;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSTA()
{
    return write(ictx.abs, A);
}

__six502_instr result_t CPU_six502::iLDX()
{
    X = ictx.imm;
    set_flags_nz(X);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSTX()
{
    return write(ictx.abs, X);
}

__six502_instr result_t CPU_six502::iLDY()
{
    Y = ictx.imm;
    set_flags_nz(Y);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSTY()
{
    return write(ictx.abs, Y);
}

__six502_instr result_t CPU_six502::iTAX()
{
    X = A;
    set_flags_nz(X);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iTXA()
{
    A = X;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iTAY()
{
    Y = A;
    set_flags_nz(Y);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iTYA()
{
    A = Y;
    set_flags_nz(A);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iTSX()
{
    X = STKP;
    set_flags_nz(X);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iTXS()
{
    STKP = X;

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iPLA()
{
    result_t res = pop_stack(&A);
    set_flags_nz(A);

    return res;
}

__six502_instr result_t CPU_six502::iPHA()
{
    return push_stack(A);
}

__six502_instr result_t CPU_six502::iPLP()
{
    result_t res = pop_stack(&STATUS);
    STATUS |= FLAG_UNUSED;

    return res;
}

__six502_instr result_t CPU_six502::iPHP()
{
    u8 st = STATUS | FLAG_BREAK | FLAG_UNUSED;
    STATUS &= ~FLAG_BREAK;
    STATUS &= ~FLAG_UNUSED;
    return push_stack(st);
}
