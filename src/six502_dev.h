#pragma once

#include "six502_cmn.h"
#include <string>

namespace SIX502 {

typedef uint64_t devid_t;

/* DEV_six502 is an abstract class from which it's meant that
 * six502 library users will derive their own device class for
 * each of the device they want to put on the bus */
class DEV_six502 {
public:
    DEV_six502(const std::string& name, const addr_t& from, const addr_t& to);
    virtual ~DEV_six502() = default;

    std::string name;
    addr_range_t iorange;

    devid_t id;

    /* process_read and process_write are handlers for well read and
     * write events (who woulda thought). These are meant to be
     * overridden in derived classes and thus a user can
     * basically define a behaviour of a device they need on
     * the bus
     */
    virtual result_t process_read(const addr_t& addr, databus_t& data) = 0;
    virtual result_t process_write(const addr_t& addr,
            const databus_t& data) = 0;

    /* Returns a chunk of data in a given range on a device
     * Which really means that it preforms process_read for
     * each address in the range and packs all outputs into a
     * databus_t array
     */
    result_t fetch_data(addr_range_t& range, databus_t *data, u16& num_bytes);
};

/* Example class that implements NES RAM */
class NES_MEM_six502 : public DEV_six502 {
public:
    NES_MEM_six502(const std::string& name,
            const addr_t& from, const addr_t& to);
    ~NES_MEM_six502();

private:
    databus_t mem[MEM_MAX_2KB];
    addr_t mirror(const addr_t& addr) const;

public:
    result_t process_read(const addr_t& addr, databus_t& data);
    result_t process_write(const addr_t& addr, const databus_t& data);
};

/* Example class that implements basic RAM device with an
 * address space spanning across all 64Kb
 */
class MEM_DEV_six502 : public DEV_six502 {
public:
    MEM_DEV_six502(const std::string& name,
            const addr_t& from, const addr_t& to);
    ~MEM_DEV_six502();

private:
    databus_t mem[MEM_MAX_64KB];

public:
    result_t process_read(const addr_t& addr, databus_t& data);
    result_t process_write(const addr_t& addr, const databus_t& data);
};

} /* namespace SIX502 */
