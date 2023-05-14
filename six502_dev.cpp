#include "six502_dev.h"

DEV_six502::DEV_six502(const char *name, addr_t from, addr_t to)
{
    this->name = name;
    this->id = -1;
    fill_addr_range(&iorange, from, to);
}

MEM_DEV_six502::MEM_DEV_six502(const char *name, addr_t from, addr_t to)
    : DEV_six502(name, from, to)
{
    /* No logic required here yet */
}

MEM_DEV_six502::~MEM_DEV_six502()
{
    /* No logic required here yet */
}

__always_inline __attr_const addr_t MEM_DEV_six502::mirror(addr_t addr) const
{
    return addr & MEM_SZ_2KB;
}

result_t MEM_DEV_six502::process_read(addr_t addr, databus_t *data)
{
    if (!isin_addr_range(&iorange, addr))
        return SIX502_RET_NO_RW;

    // *data = mem[mirror(addr)];
    *data = mem[addr];
    return SIX502_RET_SUCCESS;
}

result_t MEM_DEV_six502::process_write(addr_t addr, databus_t data)
{
    if (!isin_addr_range(&iorange, addr))
        return SIX502_RET_NO_RW;

    // mem[mirror(addr)] = data;
    mem[addr] = data;
    return SIX502_RET_SUCCESS;
}
