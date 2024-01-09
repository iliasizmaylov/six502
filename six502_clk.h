#ifndef _SIX502_CLK_H_
#define _SIX502_CLK_H_

#include "six502_cmn.h"

enum __CLOCK_STATE {
    CLOCK_STOPPED,
    CLOCK_STARTED,
    CLOCK_STEP_MODE,

    NR_CLOCK_STATES
};

class CLK_six502 {
public:
    CLK_six502();
    ~CLK_six502();

private:
    u64 hz;
    u8 state;

public:
    result_t set_hz(u64 new_hz);
    u64 get_hz();
    result_t emit();
    result_t start();
    result_t stop();
};

#endif  /* _SIX502_CLK_H_ */
