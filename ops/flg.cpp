#include "six502_cpu.h"

__six502_instr result_t CPU_six502::iBRK()
{
    this->irq();
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iRTI()
{
    pop_stack(&STATUS);
    pop_pc();

    PC += 1;

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iJSR()
{
    PC--;
    push_pc();
    PC = ictx.abs;

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iRTS()
{
    pop_pc();
    PC += 1;

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iJMP()
{
    PC = ictx.abs;
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iBIT()
{
    ictx.aux = A & ictx.imm;

    set_flag(FLAG_ZERO, (ictx.aux & 0x00FF) == 0);
    set_flag(FLAG_NEG, ictx.imm & FLAG_NEG);
    set_flag(FLAG_OFLOW, ictx.imm & FLAG_OFLOW);

    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iCLC()
{
    set_flag(FLAG_CARRY, false);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSEC()
{
    set_flag(FLAG_CARRY, true);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iCLD()
{
    set_flag(FLAG_DEC, false);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSED()
{
    set_flag(FLAG_DEC, true);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iCLI()
{
    set_flag(FLAG_IRQ, false);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iSEI()
{
    set_flag(FLAG_IRQ, true);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iCLV()
{
    set_flag(FLAG_OFLOW, false);
    return SIX502_RET_SUCCESS;
}

__six502_instr result_t CPU_six502::iNOP()
{
    return SIX502_RET_SUCCESS;
}
