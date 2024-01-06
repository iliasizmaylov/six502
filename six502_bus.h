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
    result_t broadcast_read(addr_t addr, databus_t *data);
    result_t broadcast_write(addr_t addr, databus_t data);

    DEV_six502 *get_device_at_addr(addr_t addr);

    result_t load_from_file_64(std::string path);

    result_t fetch_device_data(addr_range_t range, databus_t *data,
            u16 *num_bytes);

    result_t fetch_instructions(addr_t start, struct instruction_ctx *out,
            u16 count, u16 *num);
};

#endif  /* _SIX502_BUS_H_ */
