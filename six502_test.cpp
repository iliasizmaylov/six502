#include "six502_bus.h"

#include <array>
#include <fstream>

/* The following g_test_prog array represents raw memory corresponding to
 * this 6502 program:
 * |  LDX #10
 * |  STX $0000
 * |  LDX #3
 * |  STX $0001
 * |  LDY $0000
 * |  LDA #0
 * |  CLC
 * |loop:
 * |  ADC $0001
 * |  DEY
 * |  BNE loop
 * |  STA $0002
 * |  NOP
 * |  NOP
 * |  NOP
 */

static const std::array<u8, 28> g_test_prog = {
    0xA2, 0x0A, 0x8E, 0x00, 0x00, 0xA2, 0x03, 0x8E, 0x01, 0x00, 0xAC, 0x00, 0x00,
    0xA9, 0x00, 0x18, 0x6D, 0x01, 0x00, 0x88, 0xD0, 0xFA, 0x8D, 0x02, 0x00, 0xEA, 0xEA, 0xEA
};

void printcpu(CPU_six502 *cpu, std::ofstream& fout)
{
    fout << "A: " << cpu->A << "\nX: " << cpu->X << "\nY: " << cpu->Y << std::endl;
    fout << cpu->ictx.ins->readable << std::endl << std::endl;
}

int main()
{
    BUS_six502 bus;
    bus.init_cpu();

    MEM_DEV_six502 ram("RAM", 0x0000, 0xFFFF);
    bus.add_new_device(&ram);

    addr_t offset = 0x8000;

    for (const databus_t &byte : g_test_prog)
        bus.broadcast_write(offset++, byte); 

    bus.broadcast_write(0xFFFC, 0x00); 
    bus.broadcast_write(0xFFFD, 0x80);

    std::ofstream fout("six502_out.log", std::ofstream::out);
    bus.cpu->reset();

    do {
        bus.cpu->tick();
        printcpu(bus.cpu, fout);
    } while(!bus.cpu->isnop());

    return 0;
}
