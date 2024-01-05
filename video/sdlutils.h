#ifndef __SDL_UTILS_H__
#define __SDL_UTILS_H__

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>

#include <string>
#include <stdio.h>

#include <cmath>

#define KEYPOUND_FONT_WIDTH     32
#define KEYPOUND_FONT_HEIGHT    32

#define FONT_START_ASCII        33
#define FONT_END_ASCII          130

#define FONT_LINESPACE          10

#define BOX_CHAR_UPPERLEFT      123
#define BOX_CHAR_HORIZONTAL     124
#define BOX_CHAR_UPPERRIGHT     125
#define BOX_CHAR_VERTICAL       126
#define BOX_CHAR_LOWERLEFT      127
#define BOX_CHAR_LOWERRIGHT     128

#define DEFAULT_PIXELFORMAT     SDL_PIXELFORMAT_ARGB8888

class BitmapFont {
public:
    BitmapFont();
    ~BitmapFont();

private:
    SDL_Texture *texture;
    SDL_Surface *surface;

    SDL_Rect rchars[FONT_END_ASCII];

    int width;
    int height;

    int char_width;
    int char_height;

    int char_cols;
    int char_rows;

    int char_total;

    void __render_box(SDL_Renderer *rnd, SDL_Rect *__r,
            float scale, bool has_title, std::string title,
            const SDL_Color *text_color, const SDL_Color *border_color);

public:
    bool load_font_file(SDL_Renderer *rnd, SDL_Window *win, std::string path);
    bool unload();
    void set_color(int r, int g, int b);
    void set_color(const SDL_Color *clr);
    void reset_color();

    void render(SDL_Renderer *rnd, int x, int y, std::string text, float scale);
    void render_grid(SDL_Renderer *rnd, int line, int col, std::string text, float scale);
    void render_box(SDL_Renderer *rnd, SDL_Rect *r, float scale);
    void render_box_title(SDL_Renderer *rnd, std::string title,
            SDL_Rect *r, float scale);
    void render_box_title_color(SDL_Renderer *rnd, std::string title,
            SDL_Rect *r, float scale,
            const SDL_Color *text_color, const SDL_Color *border_color);

    int get_char_width();
    int get_char_height();
};

static inline int scale_size(int size, float scale)
{
    return (int)std::round(size * scale);
}

#endif
