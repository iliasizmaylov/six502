#include "six502_dbg.h"

const char *DBG_six502::__keypound_font_file = "./video/fonts/kpndf.png";
const std::string DBG_six502::__dbg_window_name = "SIX502 DEBUGGER";

const std::string DBG_six502::__interface[MARKUP_RES] = {
    "CCCMMMDDDDDD",
    "CCCMMMDDDDDD",
    "CCCMMMDDDDDD",
    "CCCMMMDDDDDD",
    "SSSMMMDDDDDD",
    "SSSMMMDDDDDD",
    "SSSMMMDDDDDD",
    "SSSMMMDDDDDD",
    "SSSMMMDDDDDD",
    "SSSMMMDDDDDD",
    "SSSMMMDDDDDD",
    "HHHHHHHHHHHH"
};

const SDL_Color DBG_six502_window::__clr[NR_DBG_CLR] = {
    {255, 255, 255, 255},   /* DBG_CLR_DEFAULT */
    {220, 150, 60, 255},    /* DBG_CLR_ORANGE || DBG_CLR_TITLE */
    {180, 75, 30, 255},     /* DBG_CLR_REDORANGE || DBG_CLR_TITLE_BORDER */
    {235, 240, 25, 255},    /* DBG_CLR_YELLOW || DBG_CLR_SELECT */
    {183, 38, 16, 255},     /* DBG_CLR_RED || DBG_CLR_ERR */
    {69, 183, 16, 255},     /* DBG_CLR_GREEN || DBG_CLR_OK */
    {172, 172, 172, 255}    /* DBG_CLR_GREY || DBG_CLR_INACT */
};

/*************************** WINDOW CLASS ****************************/

DBG_six502_window::DBG_six502_window()
{
    this->debugger = nullptr;
}

void DBG_six502_window::resize()
{
    this->viewport = {0, 0, 0, 0};
    this->inner_viewport = {0, 0, 0, 0};

    this->cursor_line = 0;
    this->cursor_col = 0;

    int x_start = -1;
    int y_start = -1;
    int x_end = -1;
    int y_end = -1;

    int cell_w = 0;
    int cell_h = 0;

    if (!this->debugger)
        return;

    for (int i = 0; i < MARKUP_RES; i++) {
        for (int j = 0; j < MARKUP_RES; j++) {
            if (x_start < 0 && DBG_six502::__interface[i][j] == this->sym)
                x_start = j;
            if ((j >= x_end || x_end < 0) && DBG_six502::__interface[i][j] == this->sym)
                x_end = j;
            if (y_start < 0 && DBG_six502::__interface[i][j] == this->sym)
                y_start = i;
            if ((i >= y_end || y_end < 0) && DBG_six502::__interface[i][j] == this->sym)
                y_end = i;
        }
    }

    cell_w = this->debugger->wspace_w / MARKUP_RES;
    cell_h = this->debugger->wspace_h / MARKUP_RES;

    this->viewport.x = CONTENT_OFFSET + TITLE_OFFSET + this->debugger->get_title_char_w() + cell_w * x_start;
    this->viewport.y = CONTENT_OFFSET + TITLE_OFFSET + this->debugger->get_title_char_h() + cell_h * y_start;
    this->viewport.w = cell_w * (x_end - x_start + 1);
    this->viewport.h = cell_h * (y_end - y_start + 1);

    this->inner_viewport.x = this->viewport.x + this->debugger->get_content_char_w() + CONTENT_OFFSET;
    this->inner_viewport.y = this->viewport.y + this->debugger->get_content_char_h() + CONTENT_OFFSET;
    this->inner_viewport.w = this->viewport.w - this->debugger->get_content_char_w() - CONTENT_OFFSET;
    this->inner_viewport.h = this->viewport.h - this->debugger->get_content_char_h() - CONTENT_OFFSET;

    this->total_lines = (int)std::round(this->inner_viewport.h / (this->debugger->get_content_char_h() +
                scale_size(FONT_LINESPACE, this->debugger->content_scale)));
    this->total_cols = (int)std::round((int)this->inner_viewport.w / (int)this->debugger->get_content_char_w());
}

DBG_six502_window::DBG_six502_window(DBG_six502 *debugger, std::string name, char sym)
{
    this->debugger = debugger;
    this->name = name;
    this->sym = sym;

    this->title_color = DBG_six502_window::__clr[DBG_CLR_DEFAULT];
    this->border_color = DBG_six502_window::__clr[DBG_CLR_INACT];

    this->resize();
}

DBG_six502_window::~DBG_six502_window()
{
    /* XXX: A no-op as of now */
}

void DBG_six502_window::draw()
{
    this->debugger->font_engine.render_box_title_color(this->debugger->dbgrnd, this->name,
            &this->viewport, this->debugger->content_scale,
            &this->title_color, &this->border_color);
    SDL_RenderSetViewport(this->debugger->dbgrnd, &this->inner_viewport);
}

void DBG_six502_window::vrenderf(const std::string fmt, va_list args)
{
    int newline_count = 0;
    int last_col = 0;
    size_t size = fmt.size() * 2;
    size_t prev_size = size;
    std::string result;
    bool isComplete = false;

    if (size <= 0)
        return;

    while (!isComplete) {
        result.resize(size);
        size = vsnprintf((char *)result.data(), prev_size, fmt.c_str(), args);

        if (size >= 0 && size < prev_size) {
            result.resize(size + 1);
            isComplete = true;
        }

        prev_size *= 2;
    }

    for (size_t i = 0; i < size; i++)
        newline_count += result[i] == '\n' ? 1 : 0;
    last_col = get_last_col(&result);

    this->debugger->font_engine.render_grid(this->debugger->dbgrnd,
            this->cursor_line, this->cursor_col, result, this->debugger->content_scale);

    this->cursor_line += newline_count;
    this->cursor_col = last_col == 0 ? 0 : this->cursor_col + last_col;
}

void DBG_six502_window::renderf(const std::string fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    this->vrenderf(fmt, args);

    va_end(args);
}

void DBG_six502_window::crenderf(SDL_Color *clr, const std::string fmt, ...)
{
    va_list args;

    this->debugger->font_engine.set_color(clr);

    va_start(args, fmt);
    this->vrenderf(fmt, args);
    va_end(args);

    this->debugger->font_engine.reset_color();
}

void DBG_six502_window::crenderf(int clr, const std::string fmt, ...)
{
    va_list args;

    if (clr >= NR_DBG_CLR)
        clr = DBG_CLR_DEFAULT;

    const SDL_Color *sdl_clr = &DBG_six502_window::__clr[clr];

    this->debugger->font_engine.set_color(sdl_clr);

    va_start(args, fmt);
    this->vrenderf(fmt, args);
    va_end(args);

    this->debugger->font_engine.reset_color();
}

void DBG_six502_wcpustate::draw() {
    BUS_six502 *bus;
    CPU_six502 *cpu;

    DBG_six502_window::draw();

    bus = this->debugger->get_bus();
    cpu = bus->cpu;

    this->crenderf(DBG_CLR_YELLOW, "OP     : %02X (%s)\n", cpu->ictx.opcode, cpu->ictx.ins->readable.c_str());
    this->renderf("ABS    : %04X\n", cpu->ictx.abs);
    this->renderf("REL    : %04X\n\n", cpu->ictx.rel);

    this->renderf("A      : %02X\n", cpu->A);
    this->renderf("X      : %02X\n", cpu->X);
    this->renderf("Y      : %02X\n\n", cpu->Y);

    this->crenderf(DBG_CLR_YELLOW, "PC     : %04X\n", cpu->PC);
    this->renderf("STKP   : %02X\n", cpu->STKP);

    this->renderf("FLAGS  : CZIDBUON\n");
    this->renderf("       : %1u%1u%1u%1u%1u%1u%1u%1u\n\n",
            !!cpu->get_flag(FLAG_CARRY),
            !!cpu->get_flag(FLAG_ZERO),
            !!cpu->get_flag(FLAG_IRQ),
            !!cpu->get_flag(FLAG_DEC),
            !!cpu->get_flag(FLAG_BREAK),
            !!cpu->get_flag(FLAG_UNUSED),
            !!cpu->get_flag(FLAG_OFLOW),
            !!cpu->get_flag(FLAG_NEG));
}

#define DISASM_BUF_SIZE     60

void DBG_six502_wdisasm::draw() {
    DBG_six502_window::draw();

    addr_t pc_addr;
    std::string dev_name;
    u16 num;

    struct instruction_ctx *cur_ins;

    BUS_six502 *bus;
    CPU_six502 *cpu;
    DEV_six502 *dev;

    bus = this->debugger->dbgbus;
    cpu = bus->cpu;

    pc_addr = cpu->PC;
    struct instruction_ctx instrs[DISASM_BUF_SIZE];

    dev = bus->get_device_at_addr(pc_addr);
    if (!dev) {
        this->crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    bus->fetch_instructions(pc_addr, instrs, this->total_lines - 5, &num);

    if (num == 0) {
        this->crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    this->renderf(" DEVICE: ");
    this->crenderf(DBG_CLR_OK, "%s", dev->name.c_str());
    this->crenderf(DBG_CLR_GREY, " [%04X : %04X]\n\n", dev->iorange.hi, dev->iorange.lo);

    this->crenderf(DBG_CLR_GREY, " ADDR  OP  INS  ADDR ABS   REL   IMM\n\n");

    for (u16 i = 0; i < num; i++) {
        cur_ins = &instrs[i];
        if (cur_ins->opaddr == cpu->PC) {
            this->crenderf(DBG_CLR_YELLOW, ">%04X: %02X", cur_ins->opaddr, cur_ins->opcode);
        } else {
            this->crenderf(DBG_CLR_GREY, " %04X: ", cur_ins->opaddr);
            this->renderf("%02X", cur_ins->opcode);
        }

        this->renderf("  %s  %s  %04X  %04X  %02X\n",
                cur_ins->ins->readable.c_str(),
                cur_ins->ins->a_readable.c_str(),
                cur_ins->abs, cur_ins->rel, cur_ins->imm);
    }
}

void DBG_six502_whelp::draw() {
    DBG_six502_window::draw();

    this->renderf("Hey! So right now there are now controls,\nbut I plan to add them so hold on out there, OK?\n");
}

void DBG_six502_wmem::draw() {
    DBG_six502_window::draw();
}

void DBG_six502_wstack::draw() {
    DBG_six502_window::draw();

    BUS_six502 *bus = this->debugger->get_bus();
    CPU_six502 *cpu = bus->cpu;

    addr_t abs_sp_start = (STKP_PAGE_RESET | STKP_ADDR_RESET);
    addr_t abs_sp = (STKP_PAGE_RESET | cpu->STKP);

    DEV_six502 *dev;
    dev = bus->get_device_at_addr(abs_sp);
    if (!dev) {
        this->crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    this->renderf(" DEVICE: ");
    this->crenderf(DBG_CLR_OK, "%s\n", dev->name.c_str());

    u16 stack_size = abs_sp_start - abs_sp;
    if (stack_size <= 0) {
        this->crenderf(DBG_CLR_GREY, "(STACK IS EMPTY)\n");
        return;
    }

    static const int bytes_per_line = 3;
    databus_t buf[MEM_MAX_256B];
    u16 fetch_size = std::min((int)stack_size, (int)((this->total_lines - 1) * bytes_per_line) - 1);
    u16 bytes_read;
    addr_range_t fetch_range;
    fill_addr_range(&fetch_range, abs_sp, abs_sp + fetch_size);

    bus->fetch_device_data(fetch_range, buf, &bytes_read);

    if (bytes_read == 0) {
        this->crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    u16 k = 0;
    for (int i = 0; i < bytes_read; i++) {
        if (k == 0)
            this->crenderf(DBG_CLR_GREY, " %04X: ", abs_sp + i);

        if (i == 0)
            this->crenderf(DBG_CLR_YELLOW, "%02X ", buf[i]);
        else
            this->renderf("%02X ", buf[i]);

        if (k == bytes_per_line - 1) {
            this->renderf("\n");
            k = 0;
        } else {
            k++;
        }
    }
}

/*************************** DEBUGGER CLASS ****************************/

inline int DBG_six502::get_content_char_w()
{
    return scale_size(this->font_engine.get_char_width(), this->content_scale);
}

inline int DBG_six502::get_content_char_h()
{
    return scale_size(this->font_engine.get_char_height(), this->content_scale);
}

inline int DBG_six502::get_title_char_w()
{
    return scale_size(this->font_engine.get_char_width(), this->title_scale);
}

inline int DBG_six502::get_title_char_h()
{
    return scale_size(this->font_engine.get_char_height(), this->title_scale);
}

inline void DBG_six502::calc_scales()
{
    int char_w, char_h;

    if (!this->dbgwin)
        return;

    SDL_GetWindowSize(this->dbgwin, &this->dbgwin_w, &this->dbgwin_h);

    this->title_scale = (float)((float)this->dbgwin_w / 2000);
    this->title_scale = this->title_scale > TITLE_BASE_SCALE ?
        TITLE_BASE_SCALE : this->title_scale;
    this->title_scale = this->title_scale < TITLE_MIN_SCALE ?
        TITLE_MIN_SCALE : this->title_scale;

    char_w = this->get_title_char_w();
    char_h = this->get_title_char_h();

    this->wspace_w = this->dbgwin_w - char_w * 2 - TITLE_OFFSET * 2 - CONTENT_OFFSET * 2;
    this->wspace_h = this->dbgwin_h - char_h * 2 - TITLE_OFFSET * 2 - CONTENT_OFFSET * 2;

    this->content_scale = (float)((float)this->dbgwin_w / 2800);
    this->content_scale = this->content_scale > CONTENT_BASE_SCALE ?
        CONTENT_BASE_SCALE : this->content_scale;
    this->content_scale = this->content_scale < CONTENT_MIN_SCALE ?
        CONTENT_MIN_SCALE : this->content_scale;

}

DBG_six502::DBG_six502()
{
    this->dbgwin = nullptr;
    this->dbgrnd = nullptr;
    this->dbgbus = nullptr;

    this->windows = nullptr;

    this->blueclr = BLUECLR_DEFAULT;
    this->bgchange_next_ticks = 0;
    this->blueclr_mod = -1;

    this->wspace_w = 0;
    this->wspace_h = 0;

    this->step_mode = true;
    this->steps_left = 0;

    this->has_custom_pc_reset = false;
}

DBG_six502::DBG_six502(SDL_Window *win, SDL_Renderer *rnd, BUS_six502 *bus)
{
    this->dbgwin = win;
    this->dbgrnd = rnd;
    this->dbgbus = bus;

    this->windows = nullptr;

    this->wspace_w = 0;
    this->wspace_h = 0;

    this->step_mode = true;
    this->steps_left = 0;

    has_custom_pc_reset = false;

    if (!win)
        return;

    SDL_SetWindowTitle(this->dbgwin, DBG_six502::__dbg_window_name.c_str());

    this->font_engine.load_font_file(this->dbgrnd, this->dbgwin, DBG_six502::__keypound_font_file);
    this->calc_scales();

    this->windows = new DBG_six502_window* [WINDOW_COUNT];
    this->windows[0] = new DBG_six502_wcpustate(this, "CPU STATE", 'C');
    this->windows[1] = new DBG_six502_wdisasm(this, "DISASM", 'D');
    this->windows[2] = new DBG_six502_whelp(this, "HELP", 'H');
    this->windows[3] = new DBG_six502_wmem(this, "MEMORY", 'M');
    this->windows[4] = new DBG_six502_wstack(this, "STACK", 'S');

    for (int i = 0; i < WINDOW_COUNT; i++)
        this->windows[i]->resize();

    this->blueclr = BLUECLR_DEFAULT;
    this->bgchange_next_ticks = 0;
    this->blueclr_mod = -1;
}

DBG_six502::~DBG_six502()
{
    if (this->windows) {
        for (int i = 0; i < WINDOW_COUNT; i++)
            delete this->windows[i];
        delete [] this->windows;
    }
}

SDL_Window *DBG_six502::get_window()
{
    return this->dbgwin;
}

SDL_Renderer *DBG_six502::get_renderer()
{
    return this->dbgrnd;
}

BUS_six502 *DBG_six502::get_bus()
{
    return this->dbgbus;
}

result_t DBG_six502::set_window(SDL_Window *win)
{
    if (!win)
        return SIX502_RET_BAD_INPUT;

    this->dbgwin = win;
    return SIX502_RET_SUCCESS;
}

result_t DBG_six502::set_renderer(SDL_Renderer *rnd)
{
    if (!rnd)
        return SIX502_RET_BAD_INPUT;

    this->dbgrnd = rnd;
    return SIX502_RET_SUCCESS;
}

result_t DBG_six502::connect_bus(BUS_six502 *bus)
{
    if (!bus)
        return SIX502_RET_BAD_INPUT;

    this->dbgbus = bus;
    return SIX502_RET_SUCCESS;
}

void DBG_six502::draw_title_frame()
{
    SDL_Rect r = {
        .x = TITLE_OFFSET,
        .y = TITLE_OFFSET,
        .w = this->dbgwin_w - TITLE_OFFSET,
        .h = this->dbgwin_h - TITLE_OFFSET
    };

    int offset = (int)std::round((r.w / this->get_title_char_w()) / 2) - DBG_six502::__dbg_window_name.size() / 1.7;

    this->font_engine.render_box_title_color_offset(this->dbgrnd, DBG_six502::__dbg_window_name,
            offset, &r, this->title_scale,
            &DBG_six502_window::__clr[DBG_CLR_TITLE],
            &DBG_six502_window::__clr[DBG_CLR_TITLE_BORDER]);
}

void DBG_six502::draw_interface()
{
    u64 now = SDL_GetTicks64();
    SDL_Rect prev_viewport;

    if (now >= this->bgchange_next_ticks) {
        this->blueclr += this->blueclr_mod;
        if (this->blueclr >= BLUECLR_MAX || this->blueclr <= BLUECLR_MIN)
            this->blueclr_mod *= -1;

        this->bgchange_next_ticks = now + BGCHANGE_FREQUENCY;
    }

    SDL_SetRenderDrawColor(this->dbgrnd, 10, 5, blueclr, 255);
    SDL_RenderClear(this->dbgrnd);

    this->draw_title_frame();
    for (int i = 0; i < WINDOW_COUNT; i++) {
        SDL_RenderGetViewport(this->dbgrnd, &prev_viewport);

        this->windows[i]->cursor_line = 0;
        this->windows[i]->cursor_col = 0;

        this->font_engine.reset_color();

        this->windows[i]->draw();

        SDL_RenderSetViewport(this->dbgrnd, &prev_viewport);
    }
}

result_t DBG_six502::update()
{
    this->draw_interface();
    return SIX502_RET_SUCCESS;
}

void DBG_six502::step_mode_on()
{
    this->step_mode = true;
    this->steps_left = 0;
}

void DBG_six502::step_mode_off()
{
    this->step_mode = false;
    this->steps_left = 0;
}

void DBG_six502::step_mode_toggle()
{
    this->step_mode = !this->step_mode;
    this->steps_left = 0;
}

void DBG_six502::step()
{
    if (this->step_mode)
        this->steps_left++;
}

void DBG_six502::set_custom_pc_reset(addr_t pc)
{
    this->custom_pc_reset = pc;
    this->has_custom_pc_reset = true;
}

void DBG_six502::unset_custom_pc_reset()
{
    this->has_custom_pc_reset = false;
}

void DBG_six502::cpu_reset()
{
    this->dbgbus->cpu->reset();
    if (this->has_custom_pc_reset)
        this->dbgbus->cpu->PC = this->custom_pc_reset;

    this->step_mode_on();
}

result_t DBG_six502::run_cpu()
{
    if (!this->step_mode) {
        this->dbgbus->cpu->tick();
        return SIX502_RET_SUCCESS;
    }

    while (this->steps_left > 0) {
        this->dbgbus->cpu->runop_dbg();
        this->steps_left -= this->steps_left == 0 ? 0 : 1;
    }

    return SIX502_RET_SUCCESS;
}

result_t DBG_six502::process_event(SDL_Event *ev)
{
    if (ev->type == SDL_WINDOWEVENT) {
        if (ev->window.event == SDL_WINDOWEVENT_RESIZED) {
            this->calc_scales();
            for (int i = 0; i < WINDOW_COUNT; i++)
                this->windows[i]->resize();
        }
    } else if (ev->type == SDL_KEYDOWN) {
        switch (ev->key.keysym.sym) {
        case SDLK_r:
            this->cpu_reset();
            break;

        case SDLK_b:
            this->step_mode_toggle();
            break;

        case SDLK_SPACE:
            this->step();
            break;
        }
    }

    return SIX502_RET_SUCCESS;
}

