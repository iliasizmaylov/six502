#ifndef _SIX502_BUS_H_
#define _SIX502_BUS_H_

#include "six502_cpu.h"
#include "six502_dev.h"

#include <unordered_set>

class BUS_six502 {
public:
    BUS_six502();
    ~BUS_six502();

private:
    std::unordered_set<DEV_six502*> devices;

public:
    CPU_six502 *cpu;

    result_t add_new_device(DEV_six502* new_device);
    result_t init_cpu();
    result_t broadcast_read(addr_t addr, databus_t *data) const;
    result_t broadcast_write(addr_t addr, databus_t data) const;
};

#endif  /* _SIX502_BUS_H_ */
