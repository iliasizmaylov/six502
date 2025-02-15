#include "six502_cpu.h"

namespace SIX502 {

result_t CPU_six502::aIMP()
{
	ictx.abs = PC;	/* For cleaner disassemby */
	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aIMM()
{
	ictx.abs = PC;
	read(ictx.abs, ictx.imm);
	PC++;

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aZP0()
{
	read(PC, ictx.aux81);
	ictx.abs = (u16)ictx.aux81 & 0x00FF;
	read(ictx.abs, ictx.imm);
	PC++;

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aZPX()
{
	read(PC, ictx.aux81);
	ictx.aux81 += X;
	ictx.abs = (u16)ictx.aux81 & 0x00FF;
	read(ictx.abs, ictx.imm);
	PC++;

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aZPY()
{
	read(PC, ictx.aux81);
	ictx.aux81 += Y;
	ictx.abs = ictx.aux81 & 0x00FF;
	read(ictx.abs, ictx.imm);
	PC++;

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aREL()
{
	read(PC++, ictx.aux81);
	ictx.rel = (u16)ictx.aux81;
	if (ictx.rel & 0x0080)
		ictx.rel |= 0xFF00;

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aABS()
{
	read(PC++, ictx.aux81);
	read(PC++, ictx.aux82);

	ictx.abs = MERGE16(ictx.aux82, ictx.aux81);
	read(ictx.abs, ictx.imm);

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aABX()
{
	aABS();
	addr_t abs_prev = ictx.abs;
	ictx.abs += X;

	if (HI8(ictx.abs) != HI8(abs_prev))
		busy_ticks = 1;

	read(ictx.abs, ictx.imm);

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aAXR()
{
	return aABX();
}

result_t CPU_six502::aABY()
{
	aABS();
	addr_t abs_prev = ictx.abs;
	ictx.abs += Y;

	if (HI8(ictx.abs) != HI8(abs_prev))
		busy_ticks = 1;

	read(ictx.abs, ictx.imm);

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aAYR()
{
	return aABY();
}

result_t CPU_six502::aIND()
{
	read(PC++, ictx.aux81);
	read(PC++, ictx.aux82);

	addr_t ind_ptr = MERGE16(ictx.aux82, ictx.aux81);

	read(ind_ptr, ictx.aux81);

	/* This behavior is caused by a bug in the hardware */
	if (LO8(ind_ptr) == 0x00FF)
		read(ind_ptr & 0xFF00, ictx.aux82);
	else
		read(ind_ptr + 1, ictx.aux82);

	ictx.abs = MERGE16(ictx.aux82, ictx.aux81);
	read(ictx.abs, ictx.imm);

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aIZX()
{
	read(PC++, ictx.aux81);

	read((u16)(X + ictx.aux81) & 0x00FF, ictx.aux82);
	read((u16)(X + ictx.aux81 + 1) & 0x00FF, ictx.aux83);

	ictx.abs = MERGE16(ictx.aux83, ictx.aux82);
	read(ictx.abs, ictx.imm);

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aIZY()
{
	read(PC++, ictx.aux81);

	read((u16)ictx.aux81 & 0x00FF, ictx.aux82);
	read((u16)(ictx.aux81 + 1) & 0x00FF, ictx.aux83);

	addr_t abs_prev = ictx.abs;
	ictx.abs = MERGE16(ictx.aux83, ictx.aux82) + Y;
	read(ictx.abs, ictx.imm);

	if (HI8(ictx.abs) != HI8(abs_prev))
		busy_ticks += 1;

	return SIX502_RET_SUCCESS;
}

result_t CPU_six502::aIYR()
{
	return aIZY();
}

} /* namespace SIX502 */
