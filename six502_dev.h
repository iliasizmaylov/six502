#ifndef _SIX502_DEV_H_
#define _SIX502_DEV_H_

#include "six502_cmn.h"
#include <string>

typedef uint64_t devid_t;

class DEV_six502 {
public:
    DEV_six502(const char *name, addr_t from, addr_t to);
    virtual ~DEV_six502() = default;

    std::string name;
    addr_range_t iorange;

    devid_t id;
    
    virtual result_t process_read(addr_t addr, databus_t *data) = 0;
    virtual result_t process_write(addr_t addr, databus_t data) = 0;

    result_t fetch_data(addr_range_t range, 
            databus_t *data, u16 *num_bytes);
};

class MEM_DEV_six502 : public DEV_six502 {
public:
    MEM_DEV_six502(const char *name, addr_t from, addr_t to);
    ~MEM_DEV_six502();

private:
    u8 mem[MEM_MAX_2KB];
    addr_t mirror(addr_t addr) const;

public:
    result_t process_read(addr_t addr, databus_t *data);
    result_t process_write(addr_t addr, databus_t data);
};

#endif  /* _SIX502_DEV_H_ */
