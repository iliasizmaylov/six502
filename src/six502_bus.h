#pragma once

#include "six502_cpu.h"
#include "six502_dev.h"

#include <unordered_set>

namespace SIX502 {

class BUS_six502 {
public:
    BUS_six502();
    ~BUS_six502();

private:
    /* Storage for all devices on the bus
     * The reason why it is an unordered map is because when
     * six502_cpu is doing a read or a write it is oblivious
     * to the target device and only knows what address to put
     * on a databus
     * So that's why there are methods broadcast_read and
     * broadcast_write that accept an address and a buffer
     * and then just iterate over all devices and call a
     * DEV_six502::process_read or DEV_six502::process_write
     */
    std::unordered_set<DEV_six502*> devices;

public:
    CPU_six502 *cpu;

    /* Attach a new device to a bus */
    result_t add_new_device(DEV_six502* new_device);

    /* Set up the cpu */
    result_t init_cpu();

    /* Broadcast a read request from a cpu on a given address */
    result_t broadcast_read(const addr_t& addr, databus_t& data);

    /* Broadcast a write request from a cpu on a given address */
    result_t broadcast_write(const addr_t& addr, const databus_t& data);

    /* Returens a pointer to device class based on a memory address
     * if there is a device corresponding to such address
     * If there are multiple devices that are "listening" on a same address
     * (which is toatlly possible) this function will return the first
     * one it finds, which is a problem, so:
     *
     * TODO: make a method that will return vector of
     * devices that correspond to a given address
     */
    DEV_six502 *get_device_at_addr(const addr_t& addr);

    /* Load 64Kb of memory from a binary file and broadcast it across
     * all devices which basically means to fill the total 6502
     * addressible space with data from a given file */
    result_t load_from_file_64(const std::string& path);

    /* Get a bunch of bytes from a device(s) corresponding to a given address
     * range
     */
    result_t fetch_device_data(addr_range_t& range,
            databus_t *data, u16& num_bytes);

    /* The same as fetch_device_data but the output is an array of
     * instruction_ctx structs which represent an info about a disassembly of
     * data on a given address range
     */
    result_t fetch_instructions(addr_t start, struct instruction_ctx *out,
            const u16& count, u16& num);
};

} /* namepsace SIX502 */
