#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "common.h"

#define TEXTBOX_MAX_BUFFER_LENGTH (1023)
#define TEXTBOX_INITIAL_FONT_SCALE (2.0)
#define TEXTBOX_MIN_FONT_SCALE	(1.0)
#define TEXTBOX_TAB_SIZE (8)
#define TEXTBOX_MAX_HISTORY (1000)
#define TEXTBOX_CLICK_THRESHOLD_MS (350)
#define TEXTBOX_BLINK_INTERVAL (500)


typedef struct TextboxHistory TextboxHistory;
typedef struct Textbox Textbox;

struct TextboxHistory {
	char	buffer[TEXTBOX_MAX_BUFFER_LENGTH+1];
	int		length;
	int		cursor_idx;
};

struct Textbox {
	// buffer data
	char	buffer[TEXTBOX_MAX_BUFFER_LENGTH+1];
	char	wrap_buffer[TEXTBOX_MAX_BUFFER_LENGTH+1];
	int		length;

	// appearance
	int		x;
	int		y;
	int		w;
	int		h;
	RGBA 	fg_color;
	RGBA	bg_color;
	RGBA	sel_fg_color;
	RGBA	sel_bg_color;

	// spacing
	double	line_space;
	double	font_scale;
	double	padding;

	// typing cursor
	int cursor_idx;	
	int cursor_row;
	int cursor_max_col;
	int cursor_col;
	int sel_start;

	// blinking cursor
	double	last_blink_ms;
	bool	cursor_display;

	// mouse state
	bool mouse_down_prev;
	bool is_dragging;
	int	last_click_idx;
	int last_click_time_ms;
	int click_cnt;

	// history
	TextboxHistory* history;
	int history_cnt;
	int history_idx;

};


Textbox	textbox_create(int x, int y, int w, int h);
void	textbox_update(Textbox* this, Context* ctx, Event* e);
void	textbox_render(Textbox* this, Renderer* renderer);
void	textbox_syncWrapBuffer(Textbox* this);
void	textbox_pushHistory(Textbox* this);
bool	textbox_coordInTextbox(Textbox* this, double x, double y);
int		textbox_cursorIdxFromCoord(Textbox* this, double x, double y);


void textbox_getCursorRowCol(
	Textbox*	this,
	int 		cursor_idx,
	int* 		cursor_row,
	int*		cursor_col
);


#endif
