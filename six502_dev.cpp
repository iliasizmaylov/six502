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

result_t DEV_six502::fetch_data(addr_range_t range, databus_t *data, u16 *num_bytes)
{
    if (!data || !num_bytes)
        return SIX502_RET_BAD_INPUT;

    databus_t buf;
    *num_bytes = 0;
    addr_range_squeeze(&iorange, &range);

    for (addr_t i = range.hi, n = 0; i <= range.lo; i++, n++) {
        if (process_read(i, &buf) != SIX502_RET_SUCCESS)
            return SIX502_RET_NO_RW;
        data[n] = buf;
        *num_bytes += 1;
    }

    if (*num_bytes == 0)
        return SIX502_RET_NO_RW;

    return SIX502_RET_SUCCESS;
}

__always_inline __attr_const addr_t MEM_DEV_six502::mirror(addr_t addr) const
{
    return addr & MEM_SZ_2KB;
}

result_t MEM_DEV_six502::process_read(addr_t addr, databus_t *data)
{
    if (!isin_addr_range(&iorange, addr))
        return SIX502_RET_NO_RW;

    *data = mem[mirror(addr)];
    return SIX502_RET_SUCCESS;
}

result_t MEM_DEV_six502::process_write(addr_t addr, databus_t data)
{
    if (!isin_addr_range(&iorange, addr))
        return SIX502_RET_NO_RW;

    mem[mirror(addr)] = data;
    return SIX502_RET_SUCCESS;
}
