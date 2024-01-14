#ifndef _SIX502_DBG_H_
#define _SIX502_DBG_H_

#include "video/sdlutils.h"

#include "six502_cmn.h"
#include "six502_bus.h"

#include <stdarg.h>
#include <vector>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

/* Scales for title borders and text */
#define TITLE_BASE_SCALE        0.8
#define TITLE_MIN_SCALE         0.2

/* Debugger title and content offsets from screen edges */
#define TITLE_OFFSET            5
#define CONTENT_OFFSET          3

/* Debugger contents scale */
#define CONTENT_BASE_SCALE      0.5
#define CONTENT_MIN_SCALE       0.18

/* Frequency of six502 deubbger background fading in and out */
#define BGCHANGE_FREQUENCY      50

/* Starting color of six502 debugger background */
#define BLUECLR_DEFAULT         100

/* Minimum color until which background fades out */
#define BLUECLR_MIN             90

/* Maximum color until which background fades in */
#define BLUECLR_MAX             105

/* Internal debugger window types */
enum __DBG_WINDOWS {
    DBGWIN_CPUSTATE = 0,    /* Window "CPUSTATE" */
    DBGWIN_DISASM,          /* Window "DISASSEMBLY" */
    DBGWIN_HELP,            /* Window "HELP" */
    DBGWIN_MEMORY,          /* Window "MEMORY" */
    DBGWIN_STACK,           /* Window "STACK" */
    NR_DBGWIN
};

/* six502 debugger internal windows layout map resolution
 * look at DBG_six502::__interface static var definition in
 * six502_dbg.cpp for details
 */
#define MARKUP_RES              12

/* Debugger colors */
enum __DBG_COLORS {
    /* White color */
    DBG_CLR_DEFAULT = 0,

    /* Orange color - used for title mostly */
    DBG_CLR_ORANGE,
    DBG_CLR_TITLE = DBG_CLR_ORANGE,

    /* Red-ish orange - used for title borders */
    DBG_CLR_REDORANGE,
    DBG_CLR_TITLE_BORDER = DBG_CLR_REDORANGE,

    /* Yellow - many uses but mostly to highlight certain text */
    DBG_CLR_YELLOW,
    DBG_CLR_SELECT = DBG_CLR_YELLOW,

    /* Red - errors, breakpoints, etc.*/
    DBG_CLR_RED,
    DBG_CLR_ERR = DBG_CLR_RED,

    /* Green - also various uses */
    DBG_CLR_GREEN,
    DBG_CLR_OK = DBG_CLR_GREEN,

    /* Grey - to make inessential things less apparent and stuff */
    DBG_CLR_GREY,
    DBG_CLR_INACT = DBG_CLR_GREY,

    NR_DBG_CLR
};

class DBG_six502;

class DBG_six502_window {
    friend class DBG_six502;

public:
    static const SDL_Color __clr[NR_DBG_CLR];

    DBG_six502_window();
    DBG_six502_window(DBG_six502 *debugger, std::string name, char sym);

    virtual ~DBG_six502_window() = 0;

protected:
    DBG_six502 *debugger;
    char sym;

    int cursor_line;
    int cursor_col;

    int total_lines;
    int total_cols;

    SDL_Color title_color;
    SDL_Color border_color;

    SDL_Rect viewport;
    SDL_Rect inner_viewport;

    void vrenderf(const std::string fmt, va_list args);

public:
    std::string name;

    void set_title_color(SDL_Color new_color);
    void set_border_color(SDL_Color new_color);

    void renderf(const std::string fmt, ...);
    void crenderf(SDL_Color *clr, const std::string fmt, ...);
    void crenderf(int clr, const std::string fmt, ...);

    void resize();
    void draw_running();
    virtual void draw();
};

class DBG_six502_wcpustate : public DBG_six502_window {
public:
    DBG_six502_wcpustate() : DBG_six502_window() {};
    DBG_six502_wcpustate(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_wcpustate() {};

    void draw() override;
};

class DBG_six502_wdisasm : public DBG_six502_window {
public:
    DBG_six502_wdisasm() : DBG_six502_window() {};
    DBG_six502_wdisasm(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_wdisasm() {};

    void draw() override;
};

class DBG_six502_whelp : public DBG_six502_window {
public:
    DBG_six502_whelp() : DBG_six502_window() {};
    DBG_six502_whelp(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_whelp() {};

    void draw() override;
};

class DBG_six502_wmem : public DBG_six502_window {
public:
    DBG_six502_wmem() : DBG_six502_window() {};
    DBG_six502_wmem(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_wmem() {};

    void draw() override;
};

class DBG_six502_wstack : public DBG_six502_window {
public:
    DBG_six502_wstack() : DBG_six502_window() {};
    DBG_six502_wstack(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_wstack() {};

    void draw() override;
};

class DBG_six502 {
    friend class DBG_six502_window;

    friend class DBG_six502_wcpustate;
    friend class DBG_six502_wdisasm;
    friend class DBG_six502_wstack;
    friend class DBG_six502_wmem;
    friend class DBG_six502_whelp;

public:
    DBG_six502();
    DBG_six502(SDL_Window *win, SDL_Renderer *rnd, BUS_six502 *bus);

    ~DBG_six502();

private:
    static const char *__keypound_font_file;
    static const std::string __dbg_window_name;
    static const std::string __interface[MARKUP_RES];

    SDL_Window *dbgwin;
    SDL_Renderer *dbgrnd;
    BUS_six502 *dbgbus;

    DBG_six502_window **windows;

    int dbgwin_w;
    int dbgwin_h;

    int wspace_w;
    int wspace_h;

    int blueclr;
    int blueclr_mod;
    u64 bgchange_next_ticks;

    std::atomic_bool step_mode;
    std::atomic_uint steps_left;
    std::atomic_bool dbg_quit;

    float title_scale;
    float content_scale;

    BitmapFont font_engine;

    addr_t custom_pc_reset;
    bool has_custom_pc_reset;

    std::vector<addr_t> breakpoints;
    std::vector<addr_t> watchpoints;

    std::mutex run_lock;
    std::thread *run_thread;
    std::condition_variable step_mode_cv;
    std::condition_variable cpu_awake_cv;
    std::atomic_bool cpu_relax_flag;
    std::chrono::nanoseconds cpu_relax_time;

    std::atomic_ullong nticks_before_relax;
    std::atomic_ullong ticks_after_relax;

    inline void calc_scales();

    inline int get_content_char_w();
    inline int get_content_char_h();

    inline int get_title_char_w();
    inline int get_title_char_h();

    void draw_title_frame();
    void draw_interface();

    result_t run_cpu();

public:
    result_t set_window(SDL_Window *win);
    result_t set_renderer(SDL_Renderer *rnd);
    result_t connect_bus(BUS_six502 *bus);

    SDL_Window *get_window();
    SDL_Renderer *get_renderer();
    BUS_six502 *get_bus();

    result_t update();
    result_t process_event(SDL_Event *ev);

    void set_breakpoints(std::vector<addr_t> breaks);
    void set_watchpoints(std::vector<addr_t> watch);

    void set_custom_pc_reset(addr_t pc);
    void unset_custom_pc_reset();
    void cpu_reset();

    void step_mode_on();
    void step_mode_off();
    void step_mode_toggle();

    void step();

    result_t run_cpu_once();
    void awake_cpu();
    void start_cpu();
    void relax_cpu();
    void relax_cpu(std::chrono::nanoseconds t);
    void relax_cpu_each_nticks(u64 nticks);
};

static inline int get_last_col(std::string *tgt)
{
    int size = 0;
    std::string entry;
    size_t pos;

    if (!tgt)
        return 0;

    size = tgt->size();
    if ((*tgt)[size - 2] == '\n')
        return 0;

    pos = tgt->rfind("\n");
    if (pos == std::string::npos) {
        return size - 1;
    } else {
        entry = tgt->substr(pos, size);
        return entry.size() - 1;
    }

    return 0;
}

#endif
