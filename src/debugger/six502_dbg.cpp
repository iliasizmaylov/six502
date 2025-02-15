#include "six502_dbg.h"

namespace SIX502 {

const char *DBG_six502::__keypound_font_file = "./src/video/fonts/kpndf.png";
const std::string DBG_six502::__dbg_window_name = "SIX502 DEBUGGER";

/* This one describes the layout of debugger windows by defining
 * a string array of size MARKUP_RES and filling it with symbols
 * with each one corresponding to it's window object and thus
 * defining layout
 *
 * The array of string represents a debugger window and each symbol
 * represents a chunk of a screen that will be dedicated to a certain
 * window
 */
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
    debugger = nullptr;
}

void DBG_six502_window::resize()
{
    viewport = {0, 0, 0, 0};
    inner_viewport = {0, 0, 0, 0};

    cursor_line = 0;
    cursor_col = 0;

    int x_start = -1;
    int y_start = -1;
    int x_end = -1;
    int y_end = -1;

    int cell_w = 0;
    int cell_h = 0;

    if (!debugger)
        return;

    for (int i = 0; i < MARKUP_RES; i++) {
        for (int j = 0; j < MARKUP_RES; j++) {
            if (x_start < 0 && DBG_six502::__interface[i][j] == sym)
                x_start = j;
            if ((j >= x_end || x_end < 0) && DBG_six502::__interface[i][j] == sym)
                x_end = j;
            if (y_start < 0 && DBG_six502::__interface[i][j] == sym)
                y_start = i;
            if ((i >= y_end || y_end < 0) && DBG_six502::__interface[i][j] == sym)
                y_end = i;
        }
    }

    cell_w = debugger->wspace_w / MARKUP_RES;
    cell_h = debugger->wspace_h / MARKUP_RES;

    viewport.x = CONTENT_OFFSET + TITLE_OFFSET + debugger->get_title_char_w() + cell_w * x_start;
    viewport.y = CONTENT_OFFSET + TITLE_OFFSET + debugger->get_title_char_h() + cell_h * y_start;
    viewport.w = cell_w * (x_end - x_start + 1);
    viewport.h = cell_h * (y_end - y_start + 1);

    inner_viewport.x = viewport.x + debugger->get_content_char_w() + CONTENT_OFFSET;
    inner_viewport.y = viewport.y + debugger->get_content_char_h() + CONTENT_OFFSET;
    inner_viewport.w = viewport.w - debugger->get_content_char_w() - CONTENT_OFFSET;
    inner_viewport.h = viewport.h - debugger->get_content_char_h() - CONTENT_OFFSET;

    total_lines = (int)std::round(inner_viewport.h / (debugger->get_content_char_h() +
                scale_size(FONT_LINESPACE, debugger->content_scale)));
    total_cols = (int)std::round((int)inner_viewport.w / (int)debugger->get_content_char_w());
}

DBG_six502_window::DBG_six502_window(DBG_six502 *debugger, std::string name, char sym)
{
    this->debugger = debugger;
    this->name = name;
    this->sym = sym;

    title_color = DBG_six502_window::__clr[DBG_CLR_DEFAULT];
    border_color = DBG_six502_window::__clr[DBG_CLR_INACT];

    resize();
}

DBG_six502_window::~DBG_six502_window()
{
    /* XXX: A no-op as of now */
}

void DBG_six502_window::draw()
{
    debugger->font_engine.render_box_title_color(debugger->dbgrnd, name,
            &viewport, debugger->content_scale,
            &title_color, &border_color);
    SDL_RenderSetViewport(debugger->dbgrnd, &inner_viewport);
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

        if (size < prev_size) {
            result.resize(size + 1);
            isComplete = true;
        }

        prev_size *= 2;
    }

    for (size_t i = 0; i < size; i++)
        newline_count += result[i] == '\n' ? 1 : 0;
    last_col = get_last_col(&result);

    debugger->font_engine.render_grid(debugger->dbgrnd,
            cursor_line, cursor_col, result, debugger->content_scale);

    cursor_line += newline_count;
    cursor_col = last_col == 0 ? 0 : cursor_col + last_col;
}

void DBG_six502_window::renderf(const std::string fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    vrenderf(fmt, args);

    va_end(args);
}

void DBG_six502_window::crenderf(SDL_Color *clr, const std::string fmt, ...)
{
    va_list args;

    debugger->font_engine.set_color(clr);

    va_start(args, fmt);
    vrenderf(fmt, args);
    va_end(args);

    debugger->font_engine.reset_color();
}

void DBG_six502_window::crenderf(int clr, const std::string fmt, ...)
{
    va_list args;

    if (clr >= NR_DBG_CLR)
        clr = DBG_CLR_DEFAULT;

    const SDL_Color *sdl_clr = &DBG_six502_window::__clr[clr];

    debugger->font_engine.set_color(sdl_clr);

    va_start(args, fmt);
    vrenderf(fmt, args);
    va_end(args);

    debugger->font_engine.reset_color();
}

void DBG_six502_window::draw_running() {
    DBG_six502_window::draw();
    crenderf(DBG_CLR_GREY, "\n CPU IS RUNNING...\n\n BREAK CPU TO\n SEE THE STATE");
}

void DBG_six502_wcpustate::draw() {
    DBG_six502_window::draw();

    std::unique_lock<std::mutex> ul_dbg_run_lock(debugger->run_lock, std::try_to_lock);
    if (!ul_dbg_run_lock.owns_lock())
        return;

    BUS_six502 *bus;
    CPU_six502 *cpu;

    bus = debugger->get_bus();
    cpu = bus->cpu;

    crenderf(DBG_CLR_YELLOW, "OP     : %02X (%s)\n", cpu->ictx.opcode, cpu->ictx.ins->readable.c_str());
    renderf("ABS    : %04X\n", cpu->ictx.abs);
    renderf("IMM    : %02X\n", cpu->ictx.imm);
    renderf("REL    : %04X\n", cpu->ictx.rel_d);
    crenderf(DBG_CLR_YELLOW, "PC     : %04X\n", cpu->PC);
    renderf("STKP   : %02X\n", cpu->STKP);
    renderf("A      : %02X\n", cpu->A);
    renderf("X      : %02X\n", cpu->X);
    renderf("Y      : %02X\n", cpu->Y);

    renderf("FLAGS  : CZIDBUON\n");
    renderf(" (%02X)  : %1u%1u%1u%1u%1u%1u%1u%1u\n\n",
            cpu->STATUS,
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

    std::unique_lock<std::mutex> ul_dbg_run_lock(debugger->run_lock, std::try_to_lock);
    if (!ul_dbg_run_lock.owns_lock())
        return;

    addr_t pc_addr;
    std::string dev_name;
    u16 num;

    struct instruction_ctx *cur_ins;

    BUS_six502 *bus;
    CPU_six502 *cpu;
    DEV_six502 *dev;

    bus = debugger->dbgbus;
    cpu = bus->cpu;

    pc_addr = cpu->PC;
    struct instruction_ctx instrs[DISASM_BUF_SIZE];

    dev = bus->get_device_at_addr(pc_addr);
    if (!dev) {
        crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    bus->fetch_instructions(pc_addr, instrs, total_lines - 5, num);

    if (num == 0) {
        crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    renderf(" DEVICE: ");
    crenderf(DBG_CLR_OK, "%s", dev->name.c_str());
    crenderf(DBG_CLR_GREY, " [%04X : %04X]\n\n", dev->iorange.hi, dev->iorange.lo);

    crenderf(DBG_CLR_GREY, " ADDR  OP  INS  ADDR ABS   REL   IMM\n\n");

    for (u16 i = 0; i < num; i++) {
        bool is_breakpoint = false;
        bool is_watchpoint = false;
        cur_ins = &instrs[i];

        for (auto it = debugger->breakpoints.begin(); it != debugger->breakpoints.end(); ++it) {
            if (*it == cur_ins->opaddr) {
                is_breakpoint = true;
                break;
            }
        }

        for (auto it = debugger->watchpoints.begin(); it != debugger->watchpoints.end(); ++it) {
            if (*it == cur_ins->abs) {
                is_watchpoint = true;
                break;
            }
        }

        if (cur_ins->opaddr == cpu->PC) {
            if (is_breakpoint)
                crenderf(DBG_CLR_RED, "@");
            else if (is_watchpoint)
                crenderf(DBG_CLR_ORANGE, "@");
            else
                crenderf(DBG_CLR_YELLOW, ">");

            crenderf(DBG_CLR_YELLOW, "%04X: %02X", cur_ins->opaddr, cur_ins->opcode);
        } else {
            if (is_breakpoint)
                crenderf(DBG_CLR_RED, "@");
            else if (is_watchpoint)
                crenderf(DBG_CLR_ORANGE, "@");
            else
                renderf(" ");

            crenderf(DBG_CLR_GREY, "%04X: ", cur_ins->opaddr);

            renderf("%02X", cur_ins->opcode);
        }

        renderf("  %s  %s  %04X  %04X  %02X\n",
                cur_ins->ins->readable.c_str(),
                cur_ins->ins->a_readable.c_str(),
                cur_ins->abs, cur_ins->rel_d, cur_ins->imm);
    }
}

void DBG_six502_whelp::draw() {
    DBG_six502_window::draw();

    crenderf(DBG_CLR_ORANGE, "[Q]");
    renderf("UIT  ");
    crenderf(DBG_CLR_ORANGE, "[R]");
    renderf("ESTART  ");
    crenderf(DBG_CLR_ORANGE, "[B]");
    renderf("REAK/CONTINUE  ");
    crenderf(DBG_CLR_ORANGE, "[SPACE] ");
    renderf("DEBUGGER STEP");
}

void DBG_six502_wmem::draw() {
    DBG_six502_window::draw();

    std::unique_lock<std::mutex> ul_dbg_run_lock(debugger->run_lock, std::try_to_lock);
    if (!ul_dbg_run_lock.owns_lock())
        return;

    BUS_six502 *bus = debugger->get_bus();
    CPU_six502 *cpu = bus->cpu;

    DEV_six502 *dev;
    dev = bus->get_device_at_addr(cpu->PC);
    if (!dev) {
        crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    renderf(" DEVICE: ");
    crenderf(DBG_CLR_OK, "%s\n", dev->name.c_str());

    static const int bytes_per_line = 4;
    databus_t buf[MEM_MAX_256B];
    int mem_til_end = dev->iorange.lo - cpu->PC;

    if (mem_til_end < 0) {
        crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    u16 fetch_size = std::min(mem_til_end, (int)((total_lines - 1) * bytes_per_line) - 1);
    u16 bytes_read;
    addr_range_t fetch_range;
    fill_addr_range(fetch_range, cpu->PC, cpu->PC + fetch_size);

    bus->fetch_device_data(fetch_range, buf, bytes_read);

    if (bytes_read == 0) {
        crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    u16 k = 0;
    for (int i = 0; i < bytes_read; i++) {
        if (k == 0)
            crenderf(DBG_CLR_GREY, "%04X: ", cpu->PC + i);

        if (i == 0)
            crenderf(DBG_CLR_YELLOW, "%02X ", buf[i]);
        else
            renderf("%02X ", buf[i]);

        if (k == bytes_per_line - 1) {
            renderf("\n");
            k = 0;
        } else {
            k++;
        }
    }
}

void DBG_six502_wstack::draw() {
    DBG_six502_window::draw();

    std::unique_lock<std::mutex> ul_dbg_run_lock(debugger->run_lock, std::try_to_lock);
    if (!ul_dbg_run_lock.owns_lock())
        return;

    BUS_six502 *bus = debugger->get_bus();
    CPU_six502 *cpu = bus->cpu;

    addr_t abs_sp_start = (STKP_PAGE_RESET | STKP_ADDR_RESET);
    addr_t abs_sp = (STKP_PAGE_RESET | cpu->STKP);

    DEV_six502 *dev;
    dev = bus->get_device_at_addr(abs_sp);
    if (!dev) {
        crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    renderf(" DEVICE: ");
    crenderf(DBG_CLR_OK, "%s\n", dev->name.c_str());

    u16 stack_size = abs_sp_start - abs_sp;
    if (stack_size <= 0) {
        crenderf(DBG_CLR_GREY, "(STACK IS EMPTY)\n");
        return;
    }

    static const int bytes_per_line = 4;
    databus_t buf[MEM_MAX_256B];
    u16 fetch_size = std::min((int)stack_size, (int)((total_lines - 1) * bytes_per_line) - 1);
    u16 bytes_read;
    addr_range_t fetch_range;
    fill_addr_range(fetch_range, abs_sp, abs_sp + fetch_size);

    bus->fetch_device_data(fetch_range, buf, bytes_read);

    if (bytes_read == 0) {
        crenderf(DBG_CLR_ERR, "\n CAN'T READ CURRENT DEVICE!\n");
        return;
    }

    u16 k = 0;
    for (int i = 0; i < bytes_read; i++) {
        if (k == 0)
            crenderf(DBG_CLR_GREY, "%04X: ", abs_sp + i);

        if (i == 0)
            crenderf(DBG_CLR_YELLOW, "%02X ", buf[i]);
        else
            renderf("%02X ", buf[i]);

        if (k == bytes_per_line - 1) {
            renderf("\n");
            k = 0;
        } else {
            k++;
        }
    }
}

/*************************** DEBUGGER CLASS ****************************/

inline int DBG_six502::get_content_char_w()
{
    return scale_size(font_engine.get_char_width(), content_scale);
}

inline int DBG_six502::get_content_char_h()
{
    return scale_size(font_engine.get_char_height(), content_scale);
}

inline int DBG_six502::get_title_char_w()
{
    return scale_size(font_engine.get_char_width(), title_scale);
}

inline int DBG_six502::get_title_char_h()
{
    return scale_size(font_engine.get_char_height(), title_scale);
}

inline void DBG_six502::calc_scales()
{
    int char_w, char_h;

    if (!dbgwin)
        return;

    SDL_GetWindowSize(dbgwin, &dbgwin_w, &dbgwin_h);

    title_scale = (float)((float)dbgwin_w / 2000);
    title_scale = title_scale > TITLE_BASE_SCALE ?
        TITLE_BASE_SCALE : title_scale;
    title_scale = title_scale < TITLE_MIN_SCALE ?
        TITLE_MIN_SCALE : title_scale;

    char_w = get_title_char_w();
    char_h = get_title_char_h();

    wspace_w = dbgwin_w - char_w * 2 - TITLE_OFFSET * 2 - CONTENT_OFFSET * 2;
    wspace_h = dbgwin_h - char_h * 2 - TITLE_OFFSET * 2 - CONTENT_OFFSET * 2;

    content_scale = (float)((float)dbgwin_w / 2800);
    content_scale = content_scale > CONTENT_BASE_SCALE ?
        CONTENT_BASE_SCALE : content_scale;
    content_scale = content_scale < CONTENT_MIN_SCALE ?
        CONTENT_MIN_SCALE : content_scale;

}

DBG_six502::DBG_six502()
{
    dbgwin = nullptr;
    dbgrnd = nullptr;
    dbgbus = nullptr;

    windows = nullptr;

    blueclr = BLUECLR_DEFAULT;
    bgchange_next_ticks = 0;
    blueclr_mod = -1;

    wspace_w = 0;
    wspace_h = 0;

    step_mode = true;
    steps_left = 0;

    has_custom_pc_reset = false;

    breakpoints = {};
    watchpoints = {};

    run_thread = nullptr;
    ticks_after_relax = 0;
    nticks_before_relax = 0;
}

DBG_six502::DBG_six502(SDL_Window *win, SDL_Renderer *rnd, BUS_six502 *bus)
{
    dbgwin = win;
    dbgrnd = rnd;
    dbgbus = bus;

    windows = nullptr;

    wspace_w = 0;
    wspace_h = 0;

    step_mode = true;
    steps_left = 0;

    has_custom_pc_reset = false;

    if (!win)
        return;

    SDL_SetWindowTitle(dbgwin, DBG_six502::__dbg_window_name.c_str());

    font_engine.load_font_file(dbgrnd, dbgwin, DBG_six502::__keypound_font_file);
    calc_scales();

    windows = new DBG_six502_window* [NR_DBGWIN];
    windows[DBGWIN_CPUSTATE] = new DBG_six502_wcpustate(this, "CPU STATE", 'C');
    windows[DBGWIN_DISASM] = new DBG_six502_wdisasm(this, "DISASM", 'D');
    windows[DBGWIN_HELP] = new DBG_six502_whelp(this, "HELP", 'H');
    windows[DBGWIN_MEMORY] = new DBG_six502_wmem(this, "MEMORY", 'M');
    windows[DBGWIN_STACK] = new DBG_six502_wstack(this, "STACK", 'S');

    for (int i = 0; i < NR_DBGWIN; i++)
        windows[i]->resize();

    blueclr = BLUECLR_DEFAULT;
    bgchange_next_ticks = 0;
    blueclr_mod = -1;

    breakpoints = {};
    watchpoints = {};

    run_thread = nullptr;
    ticks_after_relax = 0;
    nticks_before_relax = 0;
}

DBG_six502::~DBG_six502()
{
    if (windows) {
        for (int i = 0; i < NR_DBGWIN; i++)
            delete windows[i];
        delete [] windows;
    }

    dbg_quit = true;
    if (run_thread)
        run_thread->join();

    delete run_thread;
}

SDL_Window *DBG_six502::get_window()
{
    return dbgwin;
}

SDL_Renderer *DBG_six502::get_renderer()
{
    return dbgrnd;
}

BUS_six502 *DBG_six502::get_bus()
{
    return dbgbus;
}

result_t DBG_six502::set_window(SDL_Window *win)
{
    if (!win)
        return SIX502_RET_BAD_INPUT;

    dbgwin = win;
    return SIX502_RET_SUCCESS;
}

result_t DBG_six502::set_renderer(SDL_Renderer *rnd)
{
    if (!rnd)
        return SIX502_RET_BAD_INPUT;

    dbgrnd = rnd;
    return SIX502_RET_SUCCESS;
}

result_t DBG_six502::connect_bus(BUS_six502 *bus)
{
    std::unique_lock<std::mutex> ul_run_lock(run_lock);
    if (!bus)
        return SIX502_RET_BAD_INPUT;

    dbgbus = bus;
    return SIX502_RET_SUCCESS;
}

void DBG_six502::draw_title_frame()
{
    SDL_Rect r = {
        .x = TITLE_OFFSET,
        .y = TITLE_OFFSET,
        .w = dbgwin_w - TITLE_OFFSET,
        .h = dbgwin_h - TITLE_OFFSET
    };

    int offset = (int)std::round((r.w / get_title_char_w()) / 2) - DBG_six502::__dbg_window_name.size() / 1.7;

    font_engine.render_box_title_color_offset(dbgrnd, DBG_six502::__dbg_window_name,
            offset, &r, title_scale,
            &DBG_six502_window::__clr[DBG_CLR_TITLE],
            &DBG_six502_window::__clr[DBG_CLR_TITLE_BORDER]);
}

void DBG_six502::draw_interface()
{
    u64 now = SDL_GetTicks64();
    SDL_Rect prev_viewport;

    if (now >= bgchange_next_ticks) {
        blueclr += blueclr_mod;
        if (blueclr >= BLUECLR_MAX)
            blueclr_mod = -1;
        else if (blueclr <= BLUECLR_MIN)
            blueclr_mod = 2;

        bgchange_next_ticks = now + BGCHANGE_FREQUENCY;
    }

    SDL_SetRenderDrawColor(dbgrnd, 10, 5, blueclr, 255);
    SDL_RenderClear(dbgrnd);

    draw_title_frame();
    for (int i = 0; i < NR_DBGWIN; i++) {
        SDL_RenderGetViewport(dbgrnd, &prev_viewport);

        windows[i]->cursor_line = 0;
        windows[i]->cursor_col = 0;

        font_engine.reset_color();

        if (step_mode || i == DBGWIN_HELP)
            windows[i]->draw();
        else
            windows[i]->draw_running();

        SDL_RenderSetViewport(dbgrnd, &prev_viewport);
    }
}

result_t DBG_six502::update()
{
    draw_interface();
    return SIX502_RET_SUCCESS;
}

void DBG_six502::step_mode_on()
{
    step_mode = true;
    steps_left = 0;

    step_mode_cv.notify_one();
    cpu_awake_cv.notify_one();
    cpu_relax_flag = false;
}

void DBG_six502::step_mode_off()
{
    step_mode = false;
    steps_left = 0;

    step_mode_cv.notify_one();
    cpu_awake_cv.notify_one();
    cpu_relax_flag = false;
}

void DBG_six502::step_mode_toggle()
{
    step_mode = !step_mode;
    steps_left = 0;

    step_mode_cv.notify_one();
    cpu_awake_cv.notify_one();
    cpu_relax_flag = false;
}

void DBG_six502::step()
{
    if (step_mode)
        steps_left++;
    step_mode_cv.notify_one();
    cpu_awake_cv.notify_one();
    cpu_relax_flag = false;
}

void DBG_six502::set_breakpoints(std::vector<addr_t> breaks)
{
    breakpoints = breaks;
}

void DBG_six502::set_watchpoints(std::vector<addr_t> watch)
{
    watchpoints = watch;
}

void DBG_six502::set_custom_pc_reset(addr_t pc)
{
    custom_pc_reset = pc;
    has_custom_pc_reset = true;
}

void DBG_six502::unset_custom_pc_reset()
{
    has_custom_pc_reset = false;
}

void DBG_six502::cpu_reset()
{
    std::unique_lock<std::mutex> ul_run_lock(run_lock);
    dbgbus->cpu->reset();
    if (has_custom_pc_reset)
        dbgbus->cpu->PC = custom_pc_reset;

    dbgbus->cpu->dryrun();

    step_mode_on();
}

void DBG_six502::relax_cpu()
{
    if (run_thread && !step_mode)
        cpu_relax_flag = true;
}

void DBG_six502::relax_cpu(std::chrono::nanoseconds t)
{
    relax_cpu();
    cpu_relax_time = t;
}

result_t DBG_six502::run_cpu_once()
{
    if (cpu_relax_flag) {
        std::mutex relax_lock;
        std::unique_lock<std::mutex> ul_relax_lock(relax_lock);
        if (cpu_relax_time > std::chrono::nanoseconds(0))
            cpu_awake_cv.wait_for(ul_relax_lock, cpu_relax_time);
        else {
            cpu_awake_cv.wait(ul_relax_lock);
        }
        ticks_after_relax = 0;
        cpu_relax_flag = false;
    }

    std::unique_lock<std::mutex> ul_run_lock(run_lock);
    dbgbus->cpu->dryrun();

    if (!step_mode) {
        if (dbgbus->cpu->aux_state & CPU_JUMP_SELF)
            breakpoints.push_back(dbgbus->cpu->PC);

        for (auto it = breakpoints.begin(); it != breakpoints.end(); ++it) {
            if (*it == dbgbus->cpu->PC) {
                step_mode_on();
                return SIX502_RET_SUCCESS;
            }
        }

        for (auto it = watchpoints.begin(); it != watchpoints.end(); ++it) {
            if (*it == dbgbus->cpu->ictx.abs) {
                step_mode_on();
                return SIX502_RET_SUCCESS;
            }
        }

        dbgbus->cpu->tick();
        if (nticks_before_relax > 0) {
            ticks_after_relax++;

            if (ticks_after_relax >= nticks_before_relax) {
                ticks_after_relax = 0;
                relax_cpu();
            }
        }
    } else {
        ul_run_lock.unlock();
        ticks_after_relax = 0;
        std::mutex step_mode_cv_lock;
        std::unique_lock<std::mutex> ul_step_mode_cv_lock(step_mode_cv_lock);
        step_mode_cv.wait(ul_step_mode_cv_lock);
        ul_run_lock.lock();
        while (steps_left > 0) {
            dbgbus->cpu->runop_dbg();
            steps_left -= steps_left == 0 ? 0 : 1;
        }
        ul_run_lock.unlock();
    }

    return SIX502_RET_SUCCESS;
}

result_t DBG_six502::run_cpu()
{
    while (!dbg_quit)
        run_cpu_once();

    return SIX502_RET_SUCCESS;
}

void DBG_six502::awake_cpu()
{
    if (run_thread && cpu_relax_flag)
        cpu_awake_cv.notify_one();
}

void DBG_six502::start_cpu()
{
    if (!run_thread)
        run_thread = new std::thread(&DBG_six502::run_cpu, this);
    awake_cpu();
}

void DBG_six502::relax_cpu_each_nticks(u64 nticks)
{
    nticks_before_relax = nticks;
}

result_t DBG_six502::process_event(SDL_Event *ev)
{
    if (ev->type == SDL_WINDOWEVENT) {
        if (ev->window.event == SDL_WINDOWEVENT_RESIZED) {
            calc_scales();
            for (int i = 0; i < NR_DBGWIN; i++)
                windows[i]->resize();
        }
    } else if (ev->type == SDL_KEYDOWN) {
        switch (ev->key.keysym.sym) {
        case SDLK_r:
            cpu_reset();
            break;

        case SDLK_b:
            step_mode_toggle();
            break;

        case SDLK_SPACE:
            step();
            break;

        case SDLK_q:
            dbg_quit = true;
            step_mode_off();
            return SIX502_RET_QUIT;
        }
    }

    return SIX502_RET_SUCCESS;
}

} /* namespace SIX502 */
