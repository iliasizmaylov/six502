#include "sdlutils.h"

namespace SIX502 {

BitmapFont::BitmapFont()
{
	this->texture = nullptr;
	this->surface = nullptr;

	this->unload();
}

BitmapFont::~BitmapFont()
{
	this->unload();
}

bool BitmapFont::unload()
{
	if (this->texture)
		SDL_DestroyTexture(this->texture);

	if (this->surface)
		SDL_FreeSurface(this->surface);

	this->texture = nullptr;
	this->surface = nullptr;

	return true;
}

bool BitmapFont::load_font_file(SDL_Renderer *rnd, SDL_Window *win, std::string path)
{
	SDL_Surface *temp_surface = nullptr;

	if (!rnd || !win)
		return false;

	this->char_width = KEYPOUND_FONT_WIDTH;
	this->char_height = KEYPOUND_FONT_HEIGHT;

	temp_surface = IMG_Load(path.c_str());
	if (!temp_surface)
		return false;

	this->surface = SDL_ConvertSurfaceFormat(temp_surface,
			SDL_GetWindowPixelFormat(win), 0);

	if (!this->surface)
		return false;


	this->width = this->surface->w;
	this->height = this->surface->h;

	this->texture = SDL_CreateTextureFromSurface(rnd, this->surface);

	if (!this->texture)
		return false;

	SDL_SetTextureBlendMode(this->texture, SDL_BLENDMODE_BLEND);

	SDL_FreeSurface(temp_surface);
	SDL_FreeSurface(this->surface);

	this->surface = nullptr;

	this->char_cols = this->width / this->char_width;
	this->char_rows = this->height / this->char_height;

	this->char_total = this->char_cols * this->char_rows;

	if (this->char_total <= 0)
		return false;

	int cur_ascii = FONT_START_ASCII;

	for (int i = 0; i < this->char_rows; i++) {
		for (int j = 0; j < this->char_cols; j++) {
			this->rchars[cur_ascii].x = this->char_width * j;
			this->rchars[cur_ascii].y = this->char_height * i;
			this->rchars[cur_ascii].w = this->char_width;
			this->rchars[cur_ascii].h = this->char_height;

			cur_ascii++;
		}
	}

	return true;
}

void BitmapFont::set_color(int r, int g, int b)
{
	SDL_SetTextureColorMod(this->texture, r, g, b);
}

void BitmapFont::set_color(const SDL_Color *clr)
{
	SDL_SetTextureColorMod(this->texture, clr->r, clr->g, clr->b);
}

void BitmapFont::reset_color()
{
	SDL_SetTextureColorMod(this->texture, 255, 255, 255);
}

void BitmapFont::render(SDL_Renderer *rnd, int x, int y, std::string text, float scale)
{
	int cur_x = x, cur_y = y;
	size_t length;

	if (!this->texture)
		return;

	length = text.size();
	for (size_t i = 0; i < length; i++) {
		switch (text[i]) {
		case ' ':
			cur_x += scale_size(this->char_width, scale);
			break;
		case '\n':
			cur_x = x;
			cur_y += scale_size(this->char_width + FONT_LINESPACE, scale);
			break;
		default:
			int code = (int)text[i];

			if (code < FONT_START_ASCII || code > FONT_END_ASCII)
				continue;

			SDL_Rect r = {
				cur_x,
				cur_y,
				this->rchars[code].w,
				this->rchars[code].h
			};

			r.w = scale_size(r.w, scale);
			r.h = scale_size(r.h, scale);
			SDL_RenderCopy(rnd, this->texture, &this->rchars[code], &r);
			cur_x += scale_size(this->rchars[code].w, scale);
			break;
		}
	}
}

void BitmapFont::render_grid(SDL_Renderer *rnd, int line, int col, std::string text, float scale)
{
	int x, y;
	int cw, ch;

	cw = scale_size(this->char_width, scale);
	ch = scale_size(this->char_height + FONT_LINESPACE, scale);

	x = cw * col;
	y = ch * line;

	this->render(rnd, x, y, text, scale);
}

void BitmapFont::__render_box(SDL_Renderer *rnd,
		SDL_Rect *__r, float scale,
		bool has_title, int offset, std::string title,
		const SDL_Color *text_color, const SDL_Color *border_color)
{
	SDL_Rect r;

	int num_hor;
	int num_ver;

	int x = __r->x, y = __r->y, w = __r->w, h = __r->h;

	int font_w = scale_size(this->char_width, scale);
	int font_h = scale_size(this->char_height, scale);

	font_w = font_w == 0 ? 1 : font_w;
	font_h = font_h == 0 ? 1 : font_h;

	size_t title_length = 0;

	if (has_title) {
		title = " " + title + " ";
		title_length = title.size();
	}

	if (title_length <= 0)
		has_title = false;

	num_hor = (int)std::ceil((w - font_w * 2) / font_w);
	num_ver = (int)std::ceil((h - font_h * 2) / font_h);

	r.w = font_w;
	r.h = font_h;

	this->set_color(border_color);

	for (int i = 0; i < num_hor; i++) {
		r.x = x + font_w + (i * font_w);
		r.y = y;

		if (has_title && i == (num_ver < offset ? num_ver : offset)) {
			this->set_color(text_color);
			this->render(rnd, r.x, r.y, title, scale);
			i += title_length - 1;
			this->set_color(border_color);
		} else {
			SDL_RenderCopy(rnd, this->texture, &this->rchars[BOX_CHAR_HORIZONTAL], &r);
		}
	}

	for (int i = 0; i < num_hor; i++) {
		r.x = x + font_w + (i * font_w);
		r.y = y + h - font_h;

		SDL_RenderCopy(rnd, this->texture, &this->rchars[BOX_CHAR_HORIZONTAL], &r);
	}

	for (int i = 0; i < num_ver; i++) {
		r.x = x;
		r.y = y + font_h + (i * font_h);

		SDL_RenderCopy(rnd, this->texture, &this->rchars[BOX_CHAR_VERTICAL], &r);

		r.x = x + w - font_w;
		r.y = y + font_h + (i * font_h);

		SDL_RenderCopy(rnd, this->texture, &this->rchars[BOX_CHAR_VERTICAL], &r);
	}

	r.x = x;
	r.y = y;

	SDL_RenderCopy(rnd, this->texture, &this->rchars[BOX_CHAR_UPPERLEFT], &r);

	r.x = x;
	r.y = y + h - font_h;

	SDL_RenderCopy(rnd, this->texture, &this->rchars[BOX_CHAR_LOWERLEFT], &r);

	r.x = x + w - font_w;
	r.y = y;

	SDL_RenderCopy(rnd, this->texture, &this->rchars[BOX_CHAR_UPPERRIGHT], &r);

	r.x = x + w - font_w;
	r.y = y + h - font_h;

	SDL_RenderCopy(rnd, this->texture, &this->rchars[BOX_CHAR_LOWERRIGHT], &r);

	this->reset_color();
}

void BitmapFont::render_box(SDL_Renderer *rnd, SDL_Rect *r, float scale)
{
	SDL_Color text = {255, 255, 255, 255};
	SDL_Color border = {255, 255, 255, 255};

	this->__render_box(rnd, r, scale, false, DEFAULT_TITLE_OFFSET, "", &text, &border);
}

void BitmapFont::render_box_title(SDL_Renderer *rnd, std::string title,
		SDL_Rect *r, float scale)
{
	SDL_Color text = {255, 255, 255, 255};
	SDL_Color border = {255, 255, 255, 255};

	this->__render_box(rnd, r, scale, true, DEFAULT_TITLE_OFFSET, title, &text, &border);
}

void BitmapFont::render_box_title_color(SDL_Renderer *rnd, std::string title,
		SDL_Rect *r, float scale,
		const SDL_Color *text_color, const SDL_Color *border_color)
{
	this->__render_box(rnd, r, scale, true, DEFAULT_TITLE_OFFSET,
			title, text_color, border_color);
}

void BitmapFont::render_box_title_offset(SDL_Renderer *rnd, std::string title,
		int offset, SDL_Rect *r, float scale)
{
	SDL_Color text = {255, 255, 255, 255};
	SDL_Color border = {255, 255, 255, 255};

	this->__render_box(rnd, r, scale, true, offset, title, &text, &border);
}

void BitmapFont::render_box_title_color_offset(SDL_Renderer *rnd, std::string title,
		int offset, SDL_Rect *r, float scale,
		const SDL_Color *text_color, const SDL_Color *border_color)
{
	this->__render_box(rnd, r, scale, true, offset,
			title, text_color, border_color);
}

int BitmapFont::get_char_width()
{
	return this->char_width;
}

int BitmapFont::get_char_height()
{
	return this->char_height;
}

} /* namespace SIX502 */
