#ifndef CONTEXT_H
#define CONTEXT_H

#include "common.h"

#define CONTEXT_WIDTH	(int)(1200)
#define CONTEXT_HEIGHT	(int)(900)


typedef struct Context Context;

struct Context {
	GLFWwindow*	glfw_window;
	int			w;
	int			h;
};


Context context_create(int w, int h);


#endif
