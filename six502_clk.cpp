#include "six502_clk.h"
#include "six502_cpu.h"

CLK_six502::CLK_six502()
{
    cpu = nullptr;
    hz = 0;
    state = CLOCK_STOPPED;
}

CLK_six502::CLK_six502(CPU_six502 *new_cpu)
{
    cpu = new_cpu;
}

CLK_six502::~CLK_six502()
{
    /* No logic here yet */
}

result_t CLK_six502::set_hz(u64 new_hz)
{
    hz = new_hz;
    return SIX502_RET_SUCCESS;
}

u64 CLK_six502::get_hz()
{
    return hz;
}

result_t CLK_six502::connect(CPU_six502 *new_cpu)
{
    cpu = new_cpu;
    return SIX502_RET_SUCCESS;
}