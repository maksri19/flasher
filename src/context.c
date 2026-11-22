#include "modules.h"


Context context_create(int w, int h) {
	// initialize glfw
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
	if (!glfwInit()) log_exit("Failed to initialize GLFW\n");

	// set window creation hint
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// glfw window and context
	Context this = {0};
	this.w = w;
	this.h = h;

	this.glfw_window = glfwCreateWindow(this.w, this.h, PROGRAM_NAME, NULL, NULL);
	if (!this.glfw_window) {
		log_exit("Failed to create GLFW window\n");
	}

	// make opengl context and disable vsync
	glfwMakeContextCurrent(this.glfw_window);
	glfwSwapInterval(0);

	return this;
}
