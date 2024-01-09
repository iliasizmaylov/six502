#include "six502_bus.h"

#include <array>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>

#include "six502_dbg.h"

#define FREQUENCY_6502  1789773

int main()
{
    BUS_six502 bus;
    bus.init_cpu();

    MEM_DEV_six502 ram("RAM", 0x0000, 0xFFFF);
    bus.add_new_device(&ram);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *win;
    SDL_Renderer *rnd;

    int win_w, win_h;

    win = NULL;
    rnd = NULL;

    win = SDL_CreateWindow("Test",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            1024, 768, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP);

    rnd = SDL_CreateRenderer(win, -1, 0);

    DBG_six502 debugger(win, rnd, &bus);

    SDL_GetWindowSize(win, &win_w, &win_h);

    SDL_Event event;

    bus.load_from_file_64("tests/bin_files/6502_functional_test.bin");

    debugger.set_custom_pc_reset(0x0400);
    debugger.cpu_reset();

    bool done = false;
    u8 blueclr = 100;
    int blueclr_mod = -1;

    u64 next_ticks = SDL_GetTicks64() + 60;
    u64 prev_ticks = next_ticks;
    u64 next_bgchange_ticks = SDL_GetTicks64() + 50;

    debugger.relax_cpu_each_nticks(FREQUENCY_6502 / 60);
    debugger.start_cpu();
    while(!done) {
        debugger.awake_cpu();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                done = true;

            if (debugger.process_event(&event) == SIX502_RET_QUIT)
                done = true;
        }

        if (next_bgchange_ticks <= SDL_GetTicks64()) {
            if (blueclr <= 85 || blueclr >= 115)
                blueclr_mod *= -1;
            blueclr += blueclr_mod;
            next_bgchange_ticks += 50;
        }

        if (next_ticks <= SDL_GetTicks64()) {
            debugger.update();

            SDL_RenderPresent(rnd);
            
            prev_ticks = next_ticks;
            next_ticks += 60;
        }
       
        SDL_Delay(next_ticks - prev_ticks);
    }

    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(rnd);

    SDL_Quit();
    return 0;
}
