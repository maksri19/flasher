#include "modules.h"


int main(void)
{
	Context ctx = context_create(CONTEXT_WIDTH, CONTEXT_HEIGHT);
	Renderer renderer = renderer_create(&ctx);
	Event e = {0}; event_setCallback(&e, &ctx);

	// setup
	srand(time(NULL));
	Flasher flasher = flasher_create(&ctx);

	printf("Memory Occupied:\t%zu byte(s)\n", memory_getOffset());
	printf("Memory Available:\t%zu byte(s)\n", MEMORY_POOL_CAPACITY);
	fflush(stdout);


	// main loop
	while (!glfwWindowShouldClose(ctx.glfw_window))
	{
		// update
		flasher_update(&flasher, &ctx, &e);

		// render
		renderer_clear(&renderer, 0xb8b8b8FF);
		flasher_render(&flasher, &renderer);

		// present & swap buffer
		renderer_present(&renderer);
		glfwSwapBuffers(ctx.glfw_window);

		// event
		event_clear(&e);
		glfwWaitEventsTimeout(0.5);
	}

	return 0;
}

