#ifndef _SIX502_CLK_H_
#define _SIX502_CLK_H_

#include "six502_dev.h"

enum __CLOCK_STATE {
    CLOCK_STOPPED,
    CLOCK_STARTED,
    CLOCK_STEP_MODE,

    NR_CLOCK_STATES
};

class CPU_six502;

class CLK_six502 {
public:
    CLK_six502();
    CLK_six502(CPU_six502 *new_cpu);
    ~CLK_six502();

private:
    CPU_six502 *cpu;
    u64 hz;
    enum __CLOCK_STATE state;

public:
    result_t set_hz(u64 new_hz);
    u64 get_hz();
    result_t connect(CPU_six502 *new_cpu);
    result_t emit();
    result_t start();
    result_t stop();
};

class CLK_DEV_six502 : public DEV_six502 {
public:
    CLK_DEV_six502(const char *name, addr_t from, addr_t to, CLK_six502 *new_cpuclock);
    virtual ~CLK_DEV_six502() = default;

protected:
    CLK_six502 *cpuclock;

public:
    virtual result_t tick() = 0;
};

#endif  /* _SIX502_CLK_H_ */
