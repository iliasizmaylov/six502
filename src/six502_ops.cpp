#include "six502_cpu.h"

namespace SIX502 {

#define INIT_OP(code, INS, ADR, ticks)	\
	ops.insert({0x##code,				\
			{0x##code, #INS, #ADR,		\
			ticks, &CPU_six502::i##INS, \
			&CPU_six502::a##ADR}});

void CPU_six502::init_ops()
{
	INIT_OP(00,BRK,IMP,7) INIT_OP(01,ORA,IZX,6) INIT_OP(02,KIL,IMM,1)
	INIT_OP(08,PHP,IMP,3) INIT_OP(09,ORA,IMM,2) INIT_OP(0A,ASL,IMP,2)
	INIT_OP(10,BPL,REL,2) INIT_OP(11,ORA,IYR,5) INIT_OP(12,KIL,IMM,1)
	INIT_OP(18,CLC,IMP,2) INIT_OP(19,ORA,AYR,4) INIT_OP(1A,NOP,IMP,2)
	INIT_OP(20,JSR,ABS,6) INIT_OP(21,AND,IZX,6) INIT_OP(22,KIL,IMM,1)
	INIT_OP(28,PLP,IMP,4) INIT_OP(29,AND,IMM,2) INIT_OP(2A,ROL,IMP,2)
	INIT_OP(30,BMI,REL,2) INIT_OP(31,AND,IYR,5) INIT_OP(32,KIL,IMM,1)
	INIT_OP(38,SEC,IMP,2) INIT_OP(39,AND,AYR,4) INIT_OP(3A,NOP,IMP,2)
	INIT_OP(40,RTI,IMP,6) INIT_OP(41,EOR,IZX,6) INIT_OP(42,KIL,IMM,1)
	INIT_OP(48,PHA,IMP,3) INIT_OP(49,EOR,IMM,2) INIT_OP(4A,LSR,IMP,2)
	INIT_OP(50,BVC,REL,2) INIT_OP(51,EOR,IYR,5) INIT_OP(52,KIL,IMM,1)
	INIT_OP(58,CLI,IMP,2) INIT_OP(59,EOR,AYR,4) INIT_OP(5A,NOP,IMP,2)
	INIT_OP(60,RTS,IMP,6) INIT_OP(61,ADC,IZX,6) INIT_OP(62,KIL,IMM,1)
	INIT_OP(68,PLA,IMP,4) INIT_OP(69,ADC,IMM,2) INIT_OP(6A,ROR,IMP,2)
	INIT_OP(70,BVS,REL,2) INIT_OP(71,ADC,IYR,5) INIT_OP(72,KIL,IMM,1)
	INIT_OP(78,SEI,IMP,2) INIT_OP(79,ADC,AYR,4) INIT_OP(7A,NOP,IMP,2)
	INIT_OP(80,NOP,IMM,2) INIT_OP(81,STA,IZX,6) INIT_OP(82,NOP,IMM,2)
	INIT_OP(88,DEY,IMP,2) INIT_OP(89,NOP,IMM,2) INIT_OP(8A,TXA,IMP,2)
	INIT_OP(90,BCC,REL,2) INIT_OP(91,STA,IZY,6) INIT_OP(92,KIL,IMM,1)
	INIT_OP(98,TYA,IMP,2) INIT_OP(99,STA,ABY,5) INIT_OP(9A,TXS,IMP,2)
	INIT_OP(A0,LDY,IMM,2) INIT_OP(A1,LDA,IZX,6) INIT_OP(A2,LDX,IMM,2)
	INIT_OP(A8,TAY,IMP,2) INIT_OP(A9,LDA,IMM,2) INIT_OP(AA,TAX,IMP,2)
	INIT_OP(B0,BCS,REL,2) INIT_OP(B1,LDA,IYR,5) INIT_OP(B2,KIL,IMM,1)
	INIT_OP(B8,CLV,IMP,2) INIT_OP(B9,LDA,AYR,4) INIT_OP(BA,TSX,IMP,2)
	INIT_OP(C0,CPY,IMM,2) INIT_OP(C1,CMP,IZX,6) INIT_OP(C2,NOP,IMM,2)
	INIT_OP(C8,INY,IMP,2) INIT_OP(C9,CMP,IMM,2) INIT_OP(CA,DEX,IMP,2)
	INIT_OP(D0,BNE,REL,2) INIT_OP(D1,CMP,IYR,5) INIT_OP(D2,KIL,IMM,1)
	INIT_OP(D8,CLD,IMP,2) INIT_OP(D9,CMP,AYR,4) INIT_OP(DA,NOP,IMP,2)
	INIT_OP(E0,CPX,IMM,2) INIT_OP(E1,SBC,IZX,6) INIT_OP(E2,NOP,IMM,2)
	INIT_OP(E8,INX,IMP,2) INIT_OP(E9,SBC,IMM,2) INIT_OP(EA,NOP,IMP,2)
	INIT_OP(F0,BEQ,REL,2) INIT_OP(F1,SBC,IYR,5) INIT_OP(F2,KIL,IMM,1)
	INIT_OP(F8,SED,IMP,2) INIT_OP(F9,SBC,AYR,4) INIT_OP(FA,NOP,IMP,2)
	INIT_OP(03,SLO,IZX,8) INIT_OP(04,NOP,ZP0,3) INIT_OP(05,ORA,ZP0,3)
	INIT_OP(0B,ANC,IMM,2) INIT_OP(0C,NOP,ABS,4) INIT_OP(0D,ORA,ABS,4)
	INIT_OP(13,SLO,IZY,8) INIT_OP(14,NOP,ZPX,4) INIT_OP(15,ORA,ZPX,4)
	INIT_OP(1B,SLO,ABY,7) INIT_OP(1C,NOP,AXR,4) INIT_OP(1D,ORA,AXR,4)
	INIT_OP(23,RLA,IZX,8) INIT_OP(24,BIT,ZP0,3) INIT_OP(25,AND,ZP0,3)
	INIT_OP(2B,ANC,IMM,2) INIT_OP(2C,BIT,ABS,4) INIT_OP(2D,AND,ABS,4)
	INIT_OP(33,RLA,IZY,8) INIT_OP(34,NOP,ZPX,4) INIT_OP(35,AND,ZPX,4)
	INIT_OP(3B,RLA,ABY,7) INIT_OP(3C,NOP,AXR,4) INIT_OP(3D,AND,AXR,4)
	INIT_OP(43,SRE,IZX,8) INIT_OP(44,NOP,ZP0,3) INIT_OP(45,EOR,ZP0,3)
	INIT_OP(4B,ALR,IMM,2) INIT_OP(4C,JMP,ABS,3) INIT_OP(4D,EOR,ABS,4)
	INIT_OP(53,SRE,IZY,8) INIT_OP(54,NOP,ZPX,4) INIT_OP(55,EOR,ZPX,4)
	INIT_OP(5B,SRE,ABY,7) INIT_OP(5C,NOP,AXR,4) INIT_OP(5D,EOR,AXR,4)
	INIT_OP(63,RRA,IZX,8) INIT_OP(64,NOP,ZP0,3) INIT_OP(65,ADC,ZP0,3)
	INIT_OP(6B,ARR,IMM,2) INIT_OP(6C,JMP,IND,5) INIT_OP(6D,ADC,ABS,4)
	INIT_OP(73,RRA,IZY,8) INIT_OP(74,NOP,ZPX,4) INIT_OP(75,ADC,ZPX,4)
	INIT_OP(7B,RRA,ABY,7) INIT_OP(7C,NOP,AXR,4) INIT_OP(7D,ADC,AXR,4)
	INIT_OP(83,SAX,IZX,6) INIT_OP(84,STY,ZP0,3) INIT_OP(85,STA,ZP0,3)
	INIT_OP(8B,XAA,IMM,2) INIT_OP(8C,STY,ABS,4) INIT_OP(8D,STA,ABS,4)
	INIT_OP(93,AHX,IZY,6) INIT_OP(94,STY,ZPX,4) INIT_OP(95,STA,ZPX,4)
	INIT_OP(9B,TAS,ABY,5) INIT_OP(9C,SHY,AXR,5) INIT_OP(9D,STA,ABX,5)
	INIT_OP(A3,LAX,IZX,6) INIT_OP(A4,LDY,ZP0,3) INIT_OP(A5,LDA,ZP0,3)
	INIT_OP(AB,LAX,IMM,2) INIT_OP(AC,LDY,ABS,4) INIT_OP(AD,LDA,ABS,4)
	INIT_OP(B3,LAX,IYR,5) INIT_OP(B4,LDY,ZPX,4) INIT_OP(B5,LDA,ZPX,4)
	INIT_OP(BB,LAS,AYR,4) INIT_OP(BC,LDY,AXR,4) INIT_OP(BD,LDA,AXR,4)
	INIT_OP(C3,DCP,IZX,8) INIT_OP(C4,CPY,ZP0,3) INIT_OP(C5,CMP,ZP0,3)
	INIT_OP(CB,AXS,IMM,2) INIT_OP(CC,CPY,ABS,4) INIT_OP(CD,CMP,ABS,4)
	INIT_OP(D3,DCP,IZY,8) INIT_OP(D4,NOP,ZPX,4) INIT_OP(D5,CMP,ZPX,4)
	INIT_OP(DB,DCP,ABY,7) INIT_OP(DC,NOP,AXR,4) INIT_OP(DD,CMP,AXR,4)
	INIT_OP(E3,ISC,IZX,8) INIT_OP(E4,CPX,ZP0,3) INIT_OP(E5,SBC,ZP0,3)
	INIT_OP(EB,SBC,IMM,2) INIT_OP(EC,CPX,ABS,4) INIT_OP(ED,SBC,ABS,4)
	INIT_OP(F3,ISC,IZY,8) INIT_OP(F4,NOP,ZPX,4) INIT_OP(F5,SBC,ZPX,4)
	INIT_OP(FB,ISC,ABY,7) INIT_OP(FC,NOP,AXR,4) INIT_OP(FD,SBC,AXR,4)
	INIT_OP(06,ASL,ZP0,5) INIT_OP(07,SLO,ZP0,5)
	INIT_OP(0E,ASL,ABS,6) INIT_OP(0F,SLO,ABS,6)
	INIT_OP(16,ASL,ZPX,6) INIT_OP(17,SLO,ZPX,6)
	INIT_OP(1E,ASL,ABX,7) INIT_OP(1F,SLO,ABX,7)
	INIT_OP(26,ROL,ZP0,5) INIT_OP(27,RLA,ZP0,5)
	INIT_OP(2E,ROL,ABS,6) INIT_OP(2F,RLA,ABS,6)
	INIT_OP(36,ROL,ZPX,6) INIT_OP(37,RLA,ZPX,6)
	INIT_OP(3E,ROL,ABX,7) INIT_OP(3F,RLA,ABX,7)
	INIT_OP(46,LSR,ZP0,5) INIT_OP(47,SRE,ZP0,5)
	INIT_OP(4E,LSR,ABS,6) INIT_OP(4F,SRE,ABS,6)
	INIT_OP(56,LSR,ZPX,6) INIT_OP(57,SRE,ZPX,6)
	INIT_OP(5E,LSR,ABX,7) INIT_OP(5F,SRE,ABX,7)
	INIT_OP(66,ROR,ZP0,5) INIT_OP(67,RRA,ZP0,5)
	INIT_OP(6E,ROR,ABS,6) INIT_OP(6F,RRA,ABS,6)
	INIT_OP(76,ROR,ZPX,6) INIT_OP(77,RRA,ZPX,6)
	INIT_OP(7E,ROR,ABX,7) INIT_OP(7F,RRA,ABX,7)
	INIT_OP(86,STX,ZP0,3) INIT_OP(87,SAX,ZP0,3)
	INIT_OP(8E,STX,ABS,4) INIT_OP(8F,SAX,ABS,4)
	INIT_OP(96,STX,ZPY,4) INIT_OP(97,SAX,ZPY,2)
	INIT_OP(9E,SHX,AYR,5) INIT_OP(9F,AHX,ABY,5)
	INIT_OP(A6,LDX,ZP0,3) INIT_OP(A7,LAX,ZP0,3)
	INIT_OP(AE,LDX,ABS,4) INIT_OP(AF,LAX,ABS,4)
	INIT_OP(B6,LDX,ZPY,4) INIT_OP(B7,LAX,ZPY,4)
	INIT_OP(BE,LDX,AYR,4) INIT_OP(BF,LAX,AYR,4)
	INIT_OP(C6,DEC,ZP0,5) INIT_OP(C7,DCP,ZP0,5)
	INIT_OP(CE,DEC,ABS,6) INIT_OP(CF,DCP,ABS,6)
	INIT_OP(D6,DEC,ZPX,6) INIT_OP(D7,DCP,ZPX,6)
	INIT_OP(DE,DEC,ABX,7) INIT_OP(DF,DCP,ABX,7)
	INIT_OP(E6,INC,ZP0,5) INIT_OP(E7,ISC,ZP0,5)
	INIT_OP(EE,INC,ABS,6) INIT_OP(EF,ISC,ABS,6)
	INIT_OP(F6,INC,ZPX,6) INIT_OP(F7,ISC,ZPX,6)
	INIT_OP(FE,INC,ABX,7) INIT_OP(FF,ISC,ABX,7)
}

} /* namespace SIX502 */
