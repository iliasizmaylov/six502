#include "six502_bus.h"

#include <array>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "video/sdlutils.h"
#include "six502_dbg.h"

/* The following g_test_prog array represents raw memory corresponding to
 * this 6502 program:
 * |  LDX #10
 * |  STX $0000
 * |  LDX #3
 * |  STX $0001
 * |  LDY $0000
 * |  LDA #0
 * |  CLC
 * |loop:
 * |  ADC $0001
 * |  DEY
 * |  BNE loop
 * |  STA $0002
 * |  NOP
 * |  NOP
 * |  NOP
 */

static const std::array<databus_t, 28> g_test_prog = {
    0xA2, 0x0A, 0x8E, 0x00, 0x00, 0xA2, 0x03, 0x8E, 0x01, 0x00, 0xAC, 0x00, 0x00,
    0xA9, 0x00, 0x18, 0x6D, 0x01, 0x00, 0x88, 0xD0, 0xFA, 0x8D, 0x02, 0x00, 0xEA, 0xEA, 0xEA
};

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
            1024, 768, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    rnd = SDL_CreateRenderer(win, -1, 0);

    DBG_six502 debugger(win, rnd, &bus);

    SDL_GetWindowSize(win, &win_w, &win_h);

    SDL_Event event;

    addr_t offset = 0x000F;

    for (const databus_t &byte : g_test_prog)
        bus.broadcast_write(offset++, byte);

    bus.broadcast_write(0xFFFC, 0x0F);
    bus.broadcast_write(0xFFFD, 0x00);

    bus.cpu->reset();

    bool done = false;
    u8 blueclr = 100;
    int blueclr_mod = -1;

    float title_scale = 0.0;
    float content_scale = 0.0;

    u64 next_ticks = SDL_GetTicks64() + 30;
    u64 next_bgchange_ticks = SDL_GetTicks64() + 50;
    u64 next_cpu_ticks = SDL_GetTicks64() + 60;

    title_scale = ((float)win_w / 2000) + ((float)win_h / 2000);
    title_scale = title_scale > 0.65 ? 0.65 : title_scale;

    content_scale = ((float)win_w / 3000) + ((float)win_h / 3000);
    content_scale = content_scale > 0.5 ? 0.5 : content_scale;

    while(!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                done = true;
            debugger.process_event(&event);
        }

        if (next_bgchange_ticks <= SDL_GetTicks64()) {
            if (blueclr <= 85 || blueclr >= 115)
                blueclr_mod *= -1;
            blueclr += blueclr_mod;
            next_bgchange_ticks += 50;
        }

        if (next_cpu_ticks <= SDL_GetTicks64()) {
            if (bus.cpu->ictx.opcode != 0xEA)
                bus.cpu->tick();
            next_cpu_ticks += 60;
        }

        if (next_ticks <= SDL_GetTicks64()) {
            debugger.update();

            SDL_RenderPresent(rnd);

            next_ticks += 30;
        }
    }

    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(rnd);

    SDL_Quit();
    return 0;
}
