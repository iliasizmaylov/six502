#pragma once

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

namespace SIX502 {

/* Internal debugger window types */
enum DBG_WINDOWS {
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
enum DBG_COLORS {
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

/* DBG_six502_window class describes a pseudo-window (becuase
 * it's not actually a window like that which is created by
 * SDL for example) and displays certain information
 *
 * All windows are treated as if they were ncurses windows
 * (very broadly speaking) in a sense that they only display
 * in characters from a custom bitmap font
 *
 * DBG_six502_window is an abstract class and to actually make
 * a window there needs to be a derived class that will override
 * draw and draw_running methods
 */
class DBG_six502_window {
    /* DBG_six502_window and DBG_six502 itself are friends
     * because they have to actively use one another's interfaces
     * for obvious reasons (since windows display debugger info)
     */
    friend class DBG_six502;

public:
    /* List of colors available in windows */
    static const SDL_Color __clr[NR_DBG_CLR];

    DBG_six502_window();
    DBG_six502_window(DBG_six502 *debugger, std::string name, char sym);

    virtual ~DBG_six502_window() = 0;

protected:
    /* Pointer to parent debugger class */
    DBG_six502 *debugger;

    /* Symbol which represents this window in DBG_six502::__interface
     * array which is a display map of all debugger windows within
     * debugger's own SDL window
     */
    char sym;

    /* Virtual cursor line of the current window */
    int cursor_line;

    /* Virtual cursor column of the current window */
    int cursor_col;

    /* Total virtual lines in the current window */
    int total_lines;

    /* Total virtual columns in the current window */
    int total_cols;

    /* Color used for title text */
    SDL_Color title_color;

    /* Color used for ascii draw char style borders */
    SDL_Color border_color;

    /* Viewport that encapsulates whole window (including borders) */
    SDL_Rect viewport;

    /* Viewport that encapsulates only windows' contents (within borders) */
    SDL_Rect inner_viewport;

    /* This is a function that mimics vprintf behaviour but rather
     * outputs a given formatted string into SDL window and then calculates
     * new current virtual column and line
     */
    void vrenderf(const std::string fmt, va_list args);

public:
    std::string name;

    void set_title_color(SDL_Color new_color);
    void set_border_color(SDL_Color new_color);

    /* A wrapper around DBG_six502_window::vrenderf
     * Outputs a string with a curently set color
     */
    void renderf(const std::string fmt, ...);

    /* A wrapper around DBG_six502_window::vrenderf
     * Outputs a string with a color specified as a pointer
     * to a struct SDL_Color
     */
    void crenderf(SDL_Color *clr, const std::string fmt, ...);

    /* A wrapper around DBG_six502_window::vrenderf
     * Outputs a string with a color specified as an id
     * within enum DBG_COLORS
     */
    void crenderf(int clr, const std::string fmt, ...);

    /* This method is called when the whole debugger SDL window
     * is being resized. It recalculates font sizes, borders and
     * virtual columns and lines accordingly
     */
    void resize();

    /* There are two methods that are called for each debugger window
     * when the whole debugger redraw is requested
     * Method draw_running is redrawing everything in way as if a
     * debugger is currently running (i.e. not in step mode) so
     * for example a disassembly window will display disassembly each
     * frame when the CPU to which the debugger is connected is running
     * because it would be pointless and instead it draws a simple message
     * like "cpu is running, break it to see the current state" */
    void draw_running();

    /* There are two methods that are called for each debugger window
     * when the whole debugger redraw is requested
     * Method draw is redrawing everything as if a CPU to which debugger
     * is connected is not in a running state */
    virtual void draw();
};

/* Class DBG_six502_wcpustate is derived from DBG_six502_window and
 * describes a "CPU STATE" window */
class DBG_six502_wcpustate : public DBG_six502_window {
public:
    DBG_six502_wcpustate() : DBG_six502_window() {};
    DBG_six502_wcpustate(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_wcpustate() {};

    void draw() override;
};

/* Class DBG_six502_wdisasm is derived from DBG_six502_window and
 * describes a "DISASSSEMBLY" window */
class DBG_six502_wdisasm : public DBG_six502_window {
public:
    DBG_six502_wdisasm() : DBG_six502_window() {};
    DBG_six502_wdisasm(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_wdisasm() {};

    void draw() override;
};

/* Class DBG_six502_whelp is derived from DBG_six502_window and
 * describes a "HELP" window */
class DBG_six502_whelp : public DBG_six502_window {
public:
    DBG_six502_whelp() : DBG_six502_window() {};
    DBG_six502_whelp(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_whelp() {};

    void draw() override;
};

/* Class DBG_six502_wmem is derived from DBG_six502_window and
 * describes a "MEMORY" window */
class DBG_six502_wmem : public DBG_six502_window {
public:
    DBG_six502_wmem() : DBG_six502_window() {};
    DBG_six502_wmem(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_wmem() {};

    void draw() override;
};

/* Class DBG_six502_wstack is derived from DBG_six502_window and
 * describes a "STACK" window */
class DBG_six502_wstack : public DBG_six502_window {
public:
    DBG_six502_wstack() : DBG_six502_window() {};
    DBG_six502_wstack(DBG_six502 *debugger, std::string name, char sym) :
            DBG_six502_window(debugger, name, sym) {};

    ~DBG_six502_wstack() {};

    void draw() override;
};

/* This is a debugger class that handles actually running and stopping
 * an associated CPU_six502 object but also outputs debug info
 * into it's own SDL window
 */
class DBG_six502 {
    /* DBG_six502_window and DBG_six502 itself are friends
     * because they have to actively use one another's interfaces
     * for obvious reasons (since windows display debugger info)
     */
    friend class DBG_six502_window;

    /* Naturally all derived window classes are also friends
     * Now, this doesn't look pretty and maybe I should've created
     * an abstract factory class that would spawn these windows but
     * oh well. At least it works.
     */
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
    /* Path to a bitmap font file
     * TODO: one of those "accidental C" moments
     *       change to std::string
     */
    static const char *__keypound_font_file;

    /* Debugger window name that will be assigned to an associated SDL
     * window once it's passed into debugger class
     */
    static const std::string __dbg_window_name;

    /* Debugger interface stored as an array of strings
     *
     * From six502_dbg.cpp:
     * "This one describes the layout of debugger windows by defining
     * a string array of size MARKUP_RES and filling it with symbols
     * with each one corresponding to it's window object and thus
     * defining layout
     *
     * The array of string represents a debugger window and each symbol
     * represents a chunk of a screen that will be dedicated to a certain
     * window"
     */
    static const std::string __interface[MARKUP_RES];

    /* Pointer to associated SDL_window */
    SDL_Window *dbgwin;

    /* Pointer to associated SDL_renderer*/
    SDL_Renderer *dbgrnd;

    /* Pointer to bus class that has a CPU we want to debug */
    BUS_six502 *dbgbus;

    /* Array of pointers to debugger window objects
     * TODO: one of those "accidental C" moments
     *       change to std::vector (or std::unordered_set)
     */
    DBG_six502_window **windows;

    int dbgwin_w;                   /* Window width */
    int dbgwin_h;                   /* Window height */

    int wspace_w;                   /* Working space width */
    int wspace_h;                   /* Working space height */

    /* The next three attributes are used for fading blue background */
    int blueclr;                    /* Initial blue background color value */
    int blueclr_mod;                /* Blue color modulator */
    u64 bgchange_next_ticks;        /* Ticks until next blue color modulation */

    std::atomic_bool step_mode;     /* A flag indicating if debugger is in step mode */
    std::atomic_uint steps_left;    /* Number of debugger steps currently in queue */
    std::atomic_bool dbg_quit;      /* A flag indicated if quit is requested */

    float title_scale;              /* Font scale for title (border and text) */
    float content_scale;            /* Font scale for all the contents*/

    BitmapFont font_engine;         /* Custom bitmap font engine (which sucks) */

    addr_t custom_pc_reset;         /* Custom address to which PC will be reset */
    bool has_custom_pc_reset;       /* A flag indicating if custom PC reset was set*/

    /* Vector of breakpoin addresses */
    std::vector<addr_t> breakpoints;

    /* Vector of watchpoint addresses */
    std::vector<addr_t> watchpoints;

    /* A lock that's held when debugger is in running mode */
    std::mutex run_lock;

    /* A thread under which the debugger is (or may be) run */
    std::thread *run_thread;

    /* Condition var that's notified when step mode is initiated */
    std::condition_variable step_mode_cv;

    /* Condition var that's notified when CPU was awoken */
    std::condition_variable cpu_awake_cv;

    /* A flag indicating if CPU is currently relaxing */
    std::atomic_bool cpu_relax_flag;

    /* Stores how much time the CPU has been relaxing */
    std::chrono::nanoseconds cpu_relax_time;

    /* number of ticks until cpu will be put into relaxed mode
     * needed for dbg_six502::relax_cpu_each_nticks
     */
    std::atomic_ullong nticks_before_relax;

    /* number of ticks after cpu was be awoken
     * needed for dbg_six502::relax_cpu_each_nticks
     */
    std::atomic_ullong ticks_after_relax;

    /* Recalculates all font scales according to a new associated
     * SDL_window's size
     * Called when debugger's resize is requested
     */
    inline void calc_scales();

    /* Get char width for debugger content given the current scale */
    inline int get_content_char_w();

    /* Get char height for debugger content given the current scale */
    inline int get_content_char_h();

    /* Get char width for debugger title given the current scale */
    inline int get_title_char_w();

    /* Get char height for debugger title given the current scale */
    inline int get_title_char_h();

    /* Draws internal debugger window title and borders */
    void draw_title_frame();

    /* Basically calls each of the debugger's windows' draw (or
     * draw_running) method
     */
    void draw_interface();

    /* Starts the cpu
     * This is an internal debugger method and is not supposed
     * to be called by a user
     */
    result_t run_cpu();

public:
    /* Associate a given SDL_Window with this debugger instance */
    result_t set_window(SDL_Window *win);

    /* Associate a given SDL_Renderer with this debugger instance */
    result_t set_renderer(SDL_Renderer *rnd);

    /* Associate a given BUS_six502 object with this debugger instance */
    result_t connect_bus(BUS_six502 *bus);

    SDL_Window *get_window();       /* Return debugger's SDL_Window */
    SDL_Renderer *get_renderer();   /* Return debugger's SDL_Renderer */
    BUS_six502 *get_bus();          /* Return debugger's BUS_six502 */

    /* Update debugger according to current CPU state
     * i.e. redraw everything
     */
    result_t update();

    /* Process a given SDL_event */
    result_t process_event(SDL_Event *ev);

    /* Attach new breakpoints overriding previous ones */
    void set_breakpoints(std::vector<addr_t> breaks);

    /* Attach new watchpoints overriding previous ones */
    void set_watchpoints(std::vector<addr_t> watch);

    /* Set custom address to which CPU's PC will be reset */
    void set_custom_pc_reset(addr_t pc);

    /* Unset custom PC reset address */
    void unset_custom_pc_reset();

    /* Reset the attached CPU */
    void cpu_reset();

    void step_mode_on();        /* Turn on step mode*/
    void step_mode_off();       /* Turn off step mode */
    void step_mode_toggle();    /* Toggle step mode */

    /* Do a single debugger step */
    void step();

    /* Does one cpu tick
     * This method can be called manually in a loop or it could
     * be called by a thread DBG_six502::run_thread
     */
    result_t run_cpu_once();

    /* Notifies cpu_awake_cv condition variable */
    void awake_cpu();

    /* Creates a debugger thread and calles awake_cpu */
    void start_cpu();

    /* Sets a cpu_relax_flag */
    void relax_cpu();

    /* Sets a cpu_relax_flag and also sets cpu_relax_time */
    void relax_cpu(std::chrono::nanoseconds t);

    /* Sets nticks_before_relax thus initiating debugger mode that will relax
     * the CPU after each nticks ticks until a next call to awake_cpu
     */
    void relax_cpu_each_nticks(u64 nticks);
};

/* Utility function that calculates what's the next current column will be
 * if we output the string *tgt to the screen
 */
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

} /* namespace SIX502 */
