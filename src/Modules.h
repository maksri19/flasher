#ifndef MODULES_H
#define MODULES_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>


// useful datatypes
typedef uint32_t		RGBA;
typedef struct Point2D	Point2D;
typedef struct Vec2D	Vec2D;

typedef struct Context	Context;
typedef struct Renderer	Renderer;
typedef struct Event	Event;

typedef struct Flasher	Flasher;
typedef struct Textbox	Textbox;

#define RGBA_BLACK		(RGBA)(0x000000ff)
#define RGBA_WHITE		(RGBA)(0xffffffff)
#define RGBA_LIGHT_GRAY	(RGBA)(0xb8b8b8ff)


// definition of datatypes
struct Point2D { double x, y; };
struct Vec2D { double u, v; };


#define CONTEXT_WIDTH	(int)(1200)
#define CONTEXT_HEIGHT	(int)(900)
struct Context {
	GLFWwindow*	glfw_window;
	int			w, h;
};


struct Renderer {
	GLuint		glad_tex, glad_fbo;
	RGBA*		buffer;
    int			w, h;
};


#define EVENT_MAX_FRAME_EVENT 16
struct Event {
    unsigned int chars[EVENT_MAX_FRAME_EVENT];
    struct KeyEvent {int key, scancode, action, mods;} keys[EVENT_MAX_FRAME_EVENT];
    int char_cnt, key_cnt;
	bool left_mouse_pressed, right_mouse_pressed;
};


#define TEXTBOX_MAX_STRING_LENGTH (1024)
#define TEXTBOX_CLICK_THRESOLD_MS (350)
#define TEXTBOX_BLINK_INTERVAL_MS (500)
#define TEXTBOX_INITIAL_FONT_SCALE (2.0)
#define TEXTBOX_MIN_FONT_SCALE (0.4)
#define TEXTBOX_MAX_UNDO_HISTORY (32)
#define TEXTBOX_TAB_SIZE (8)
struct Textbox {
	// focus & buffer data
	bool	focus;
	char	buffer[TEXTBOX_MAX_STRING_LENGTH+1];
	bool	wrap_buffer[TEXTBOX_MAX_STRING_LENGTH+1];
	int		len;

	// position & appearance
	int		x, y, w, h, line_space;
	double	font_scale, padding;
	RGBA 	fg_color, bg_color, sel_fg_color, sel_bg_color;
	GLFWcursor* ibeam_cursor;

	// cursor pos
	int cursor_pos;
	int sel_start;
	int cursor_col;
	int cursor_row;

	// blinking
	double	blink_interval_ms;
	double	last_blink_ms;
	bool	cursor_display;

	// mouse state
	bool	prev_mouse_down;
	bool	is_dragging;
	double	last_click_time;
	int		last_click_pos;
	int		click_cnt;

	// history
	struct History {
		char	buffer[TEXTBOX_MAX_STRING_LENGTH+1];
		int		cursor_pos;
		int		cursor_col;
		int		cursor_row;
	} history[TEXTBOX_MAX_UNDO_HISTORY];
	int history_cnt;
	int history_idx;
};


#define FLASHER_MAX_FILE_COUNT			(1024)
#define FLASHER_FILENAME_LENGTH 		(10)
#define FLASHER_FILE_PATH				"./assets/"
struct Flasher {
	// file manament
    int file_cnt;
    int curr_file;
    char files[FLASHER_MAX_FILE_COUNT][FLASHER_FILENAME_LENGTH+1];

	// current file
	FILE* fptr;

	// textbox
	Textbox textbox;
	bool textbox_enable;
};


// static function
static uint64_t get_time_ns(void) {
    uint64_t value = glfwGetTimerValue();
    uint64_t freq = glfwGetTimerFrequency();
    uint64_t seconds = value / freq;
    uint64_t remainder = value % freq;
    return (seconds * 1000000000ULL) + ((remainder * 1000000000ULL) / freq);
}

#define log_info(fmt, ...) \
    do { \
        printf("[%s:%d] ", __FILE__, __LINE__); \
        printf(fmt, ##__VA_ARGS__); \
        fflush(stdout); \
    } while(0)

#define log_exit(fmt, ...) \
    do { \
        printf("[%s:%d] ", __FILE__, __LINE__); \
        printf(fmt, ##__VA_ARGS__); \
        fflush(stdout); \
		exit(0); \
    } while(0)


// local function
void*		memory_alloc(size_t size);

int	file_getFileNameInDir(
	const char* folder_path,
	char** files,
	int max_file_cnt,
	int max_file_name_length
);

FILE* 	file_addFile(const char* file_name, int file_name_length);
void	file_readFileToBuffer(char* buffer, int buffer_size, const char* file_name, int file_name_length);
void	file_writeFileFromBuffer(const char* file_name, int file_name_length, char* buffer, int buffer_size);
void	file_deleteFile(const char* file_name, int file_name_length);


Context		context_create(int w, int h);

Renderer	renderer_create(Context* this);
void		renderer_present(Renderer* this);
void		renderer_clear(Renderer* this, RGBA color);
void		renderer_drawLine(Renderer* this, int x0, int y0, int x1, int y1, RGBA color, int thickness);
void		renderer_drawChar(Renderer* this, char ch, int x, int y, double scale, RGBA color);
void		renderer_fillRect(Renderer* this, int x, int y, int w, int h, RGBA color);

Event		event_create(Context* ctx);
void 		event_clear(Event* this);

Flasher 	flasher_create(void);
void 		flasher_update(Flasher* this, Context* ctx, Event* e);
void 		flasher_render(Flasher* this, Renderer* renderer);


Textbox		textbox_create(int x, int y, int w, int h);
void		textbox_update(Textbox* this, Context* ctx, Event* e);
void		textbox_render(Textbox* this, Renderer* renderer);


#endif
