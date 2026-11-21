#include "modules.h"


void char_callback(GLFWwindow* window, unsigned int c) {
	// get event from user pointer
    Event* e = glfwGetWindowUserPointer(window);

	// get chars from parameter
    if (e->char_cnt < EVENT_MAX_FRAME_EVENT) {
        e->chars[e->char_cnt++] = c;
    }
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// get event from user pointer
    Event* e = glfwGetWindowUserPointer(window);

	// get keys from parameter
    if (e->key_cnt < EVENT_MAX_FRAME_EVENT) {
        e->keys[e->key_cnt++] = (EventKey){
            .key = key,
            .scancode = scancode,
            .action = action,
            .mods = mods
        };
    }
}


void mousebutton_callback(GLFWwindow* window, int button, int action, int mods) {
	// get event from user pointer
    Event* e = glfwGetWindowUserPointer(window);

	// check left button
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            e->mouse_pressl = true;
        } else if (action == GLFW_RELEASE) {
            e->mouse_pressl = false;
        }
    } 

	// check right button
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            e->mouse_pressr = true;
        } else if (action == GLFW_RELEASE) {
            e->mouse_pressr = false;
        }
    }
}


void event_setCallback(Event* this, Context* ctx) {
	// set user pointer
	glfwSetWindowUserPointer(ctx->glfw_window, this);

	// locate callback function
	glfwSetKeyCallback(ctx->glfw_window, key_callback);
	glfwSetCharCallback(ctx->glfw_window, char_callback);
	glfwSetMouseButtonCallback(ctx->glfw_window, mousebutton_callback);
}


void event_clear(Event* e) {
	// clear all event
	memset(e->chars, 0, sizeof(e->chars));
	memset(e->keys, 0, sizeof(e->keys));

	// set count to 0
    e->char_cnt = 0;
    e->key_cnt = 0;
}
