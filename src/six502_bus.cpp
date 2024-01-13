#include "six502_bus.h"

#include <iostream>
#include <fstream>

BUS_six502::BUS_six502()
{
    cpu = nullptr;
    devices.clear();
}

BUS_six502::~BUS_six502()
{
    if (cpu)
        delete cpu;
}

result_t BUS_six502::add_new_device(DEV_six502 *new_device)
{
    devices.insert(new_device);
    return SIX502_RET_SUCCESS;
}

result_t BUS_six502::init_cpu()
{
    cpu = new CPU_six502();
    cpu->bind(this);

    return SIX502_RET_SUCCESS;
}

result_t BUS_six502::broadcast_read(addr_t addr, databus_t *data)
{
    result_t res = SIX502_RET_STRAY_MEM;

    for (auto it = devices.begin(); it != devices.end(); ++it)
        if ((*it)->process_read(addr, data) == SIX502_RET_SUCCESS)
            res = SIX502_RET_SUCCESS;

    return res;
}

result_t BUS_six502::broadcast_write(addr_t addr, databus_t data)
{
    result_t res = SIX502_RET_STRAY_MEM;

    for (auto it = devices.begin(); it != devices.end(); ++it)
        if ((*it)->process_write(addr, data) == SIX502_RET_SUCCESS)
            res = SIX502_RET_SUCCESS;

    return res;
}

DEV_six502 *BUS_six502::get_device_at_addr(addr_t addr)
{
    databus_t buf;

    for (auto it = devices.begin(); it != devices.end(); ++it)
        if ((*it)->process_read(addr, &buf) == SIX502_RET_SUCCESS)
            return (*it);

    return NULL;
}

result_t BUS_six502::load_from_file_64(std::string path)
{
    databus_t buf;
    char buf_c;
    addr_t addr = 0x0000;

    std::ifstream test_bin(path.c_str(), std::ios::binary);
    if (!test_bin)
        return SIX502_RET_BAD_INPUT;

    while (test_bin.get(buf_c)) {
        buf = (databus_t)buf_c;
        broadcast_write(addr, buf);
        addr++;
    }

    return SIX502_RET_SUCCESS;
}

result_t BUS_six502::fetch_device_data(addr_range_t range, databus_t *data,
        u16 *num_bytes)
{
    result_t res = SIX502_RET_STRAY_MEM;
    databus_t buf;
    addr_t test_addr = range.hi;

    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if ((*it)->process_read(test_addr, &buf) == SIX502_RET_SUCCESS) {
            res = (*it)->fetch_data(range, data, num_bytes);

            if (res == SIX502_RET_SUCCESS)
                return res;
        }
    }

    return res;
}

result_t BUS_six502::fetch_instructions(addr_t start, struct instruction_ctx *out, u16 count, u16 *num)
{
    databus_t buf;
    *num = 0;

    if (!out || count == 0)
        return SIX502_RET_BAD_INPUT;
    
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if ((*it)->process_read(start, &buf) == SIX502_RET_SUCCESS) {
            cpu->save_state();
            for (u16 i = 0; i <= count; i++) {
                if ((*it)->process_read(start, &buf) != SIX502_RET_SUCCESS) {
                    *num = i;
                    break;
                }

                cpu->ictx.ins = &cpu->ops[buf];
                cpu->ictx.opcode = buf;
                cpu->ictx.opaddr = start;
                cpu->PC = start + 1;
                (cpu->*cpu->ictx.ins->addr)();

                out[i] = cpu->ictx;
                start = cpu->PC;
            }
            cpu->load_state();

            *num = count;
            return SIX502_RET_SUCCESS;
        }
    }

    return SIX502_RET_NO_RW;
}
