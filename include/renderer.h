#ifndef RENDERER_H
#define RENDERER_H

#include "common.h"
#include "context.h"


typedef struct Renderer Renderer;

struct Renderer {
	GLuint	glad_texture;
	GLuint	glad_fbo;
	RGBA*	buffer;
	int		w;
	int		h;
};


Renderer	renderer_create(Context* ctx);
void		renderer_present(Renderer* this);
void		renderer_clear(Renderer* this, RGBA color);

void renderer_fillRect(
	Renderer* this,
	int x,
	int y,
	int w,
	int h,
	RGBA color
);

void renderer_drawLine(
	Renderer* this,
	int x0,
	int y0,
	int x1,
	int y1,
	RGBA color,
	int thickness
);

void renderer_drawChar(
	Renderer* this,
	char ch,
	int x,
	int y,
	double scale,
	RGBA color
);

#endif

