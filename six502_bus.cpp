#include "six502_bus.h"

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

__always_inline result_t BUS_six502::add_new_device(DEV_six502 *new_device)
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

result_t BUS_six502::broadcast_read(addr_t addr, databus_t *data) const
{
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if ((*it)->process_read(addr, data) == SIX502_RET_SUCCESS)
            return SIX502_RET_SUCCESS;
    }

    return SIX502_RET_STRAY_MEM;
}

result_t BUS_six502::broadcast_write(addr_t addr, databus_t data) const
{
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if ((*it)->process_write(addr, data) == SIX502_RET_SUCCESS)
            return SIX502_RET_SUCCESS;
    }

    return SIX502_RET_STRAY_MEM;
}
