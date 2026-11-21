#include "modules.h"


static void flasher_newFlasher(Flasher* this) {
	// format new file name to full name
	char new_file_name[FLASHER_FILE_NAME_LENGTH+1];
	getAlphanumString(new_file_name, FLASHER_FILE_NAME_LENGTH);

	for (int i = 0; i < this->flasher_cnt; i++) {
		if (strcmp(this->flashers + i * this->stride, new_file_name) == 0) {
			log_info("Hello\n");
			getAlphanumString(new_file_name, FLASHER_FILE_NAME_LENGTH);
			i = -1;
		}
	}

	char new_file_path[9+FLASHER_FILE_NAME_LENGTH+8+1];
	sprintf(new_file_path, "%s%s%s", FLASHER_FOLDER_PATH, new_file_name, FLASHER_FILE_EXTENSION);
	
	// create file and update variable
	file_newFile(new_file_path);

	this->flasher_cnt++;
	this->flasher_idx = this->flasher_cnt - 1;
	memcpy(this->flashers + this->flasher_idx * this->stride, new_file_name, FLASHER_FILE_NAME_LENGTH+1);
}


static void flasher_deleteFlasher(Flasher* this) {
	char file_path[9+FLASHER_FILE_NAME_LENGTH+8+1];
	sprintf(file_path, "%s%s%s", FLASHER_FOLDER_PATH, this->flashers + this->flasher_idx * this->stride, FLASHER_FILE_EXTENSION);
	
	// delete file and update variable
	file_deleteFile(file_path);

	this->flasher_cnt--;
	if (this->flasher_idx > this->flasher_cnt-1) {
		this->flasher_idx = this->flasher_cnt-1;
	} else {
		// copy last file name to current delete file name
		memcpy(this->flashers + this->flasher_idx * this->stride, this->flashers + this->flasher_cnt * this->stride, this->stride);
	}
}


static void flasher_openFlasher(Flasher* this, int flasher_idx) {
	if (flasher_idx < 0 || flasher_idx > this->flasher_cnt-1) {
		log_info("Request idx out of bounds\n");
		return;
	}
	this->flasher_idx = flasher_idx;

	// format file path
	char file_path[9+FLASHER_FILE_NAME_LENGTH+8+1];
	sprintf(file_path, "%s%s%s", FLASHER_FOLDER_PATH, this->flashers + this->stride * this->flasher_idx, FLASHER_FILE_EXTENSION);

	// read file into buffer
	this->textbox.length = file_readFileToBuffer(file_path, this->textbox.buffer, TEXTBOX_MAX_BUFFER_LENGTH);
	this->textbox.cursor_idx = this->textbox.length;

	this->textbox.history_cnt = 0;
	this->textbox.history_idx = -1;
	textbox_pushHistory(&this->textbox);


	textbox_syncWrapBuffer(&this->textbox);
	textbox_getCursorRowCol(&this->textbox, this->textbox.cursor_idx, &this->textbox.cursor_row, &this->textbox.cursor_col);
	this->textbox.cursor_max_col = this->textbox.cursor_col;
}


static void flasher_saveCurrFlasher(Flasher* this) {
	// format file path
	char file_path[9+FLASHER_FILE_NAME_LENGTH+8+1];
	sprintf(file_path, "%s%s%s", FLASHER_FOLDER_PATH, this->flashers + this->flasher_idx * this->stride, FLASHER_FILE_EXTENSION);

	// write file from buffer
	file_writeFileFromBuffer(file_path, this->textbox.buffer);
}


Flasher flasher_create(void) {
    Flasher this = {0};
	this.stride = FLASHER_FILE_NAME_LENGTH+1;
	this.flasher_cnt = file_getFileNameInDir(FLASHER_FOLDER_PATH, this.flashers, this.stride, FLASHER_MAX_FILE_COUNT, FLASHER_FILE_NAME_LENGTH);
	this.flasher_idx = this.flasher_cnt > 0 ? 0 : -1;

	this.textbox = textbox_create(20, 55, 1160, 805);
	this.textbox_focus = false;
	this.textbox_just_focus = false;

	this.mouse_down_prev = false;
	
	if (this.flasher_idx >= 0) {
		flasher_openFlasher(&this, this.flasher_idx);
	}
	return this;
}


void flasher_update(Flasher* this, Context* ctx, Event* e) {
	// shift & ctrl check
	bool shift = (glfwGetKey(ctx->glfw_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(ctx->glfw_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
	bool ctrl = (glfwGetKey(ctx->glfw_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(ctx->glfw_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
	
	// mouse event
	double mouse_x, mouse_y;
	glfwGetCursorPos(ctx->glfw_window, &mouse_x, &mouse_y);
	bool mouse_down = (glfwGetMouseButton(ctx->glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
	bool mouse_just_down = mouse_down && !this->mouse_down_prev;
	bool mouse_just_up = !mouse_down && this->mouse_down_prev;
	this->mouse_down_prev = mouse_down;

	if (mouse_just_down && textbox_coordInTextbox(&this->textbox, mouse_x, mouse_y)) {
		if (this->flasher_idx >= 0) {
			this->textbox_focus = true;
			this->textbox_just_focus = true;
			this->textbox.cursor_display = true;
			this->textbox.cursor_idx = textbox_cursorIdxFromCoord(&this->textbox, mouse_x, mouse_y);
			this->textbox.last_blink_ms = (double)getTimeNs() / 1000000.0;
			textbox_getCursorRowCol(&this->textbox, this->textbox.cursor_idx, &this->textbox.cursor_row, &this->textbox.cursor_col);
		}
	}

	// if key is pressed update the data
	for (int i = 0; i < e->key_cnt; i++) {
		if (e->keys[i].action == GLFW_PRESS)
		{
			int key = e->keys[i].key;
			if (key == GLFW_KEY_ESCAPE) {
				if (this->flasher_idx >= 0) {
					flasher_saveCurrFlasher(this);
				}
				this->textbox.sel_start = -1;
				this->textbox_focus = false;
				this->textbox.cursor_display = false;
			}

			if (!this->textbox_focus) {
				// focus on textbox
				if (key == GLFW_KEY_T || key == GLFW_KEY_I || (!shift && (key == GLFW_KEY_A)) || key == GLFW_KEY_SLASH) {
					if (this->flasher_idx >= 0) {
						this->textbox_focus = true;
						this->textbox_just_focus = true;
						this->textbox.cursor_display = true;
						this->textbox.last_blink_ms = (double)getTimeNs() / 1000000.0;
					}
				}
				// next flasher
				if (key == GLFW_KEY_LEFT) {
					if (this->flasher_idx > 0) {
						flasher_saveCurrFlasher(this);
						flasher_openFlasher(this, this->flasher_idx-1);
					}
				}
				// previous flasher
				if (key == GLFW_KEY_RIGHT) {
					if (this->flasher_idx < this->flasher_cnt-1) {
						flasher_saveCurrFlasher(this);
						flasher_openFlasher(this, this->flasher_idx+1);
					}
				}
				// random flasher
				if (shift && (key == GLFW_KEY_R)) {
					if (this->flasher_cnt > 0) {
						flasher_saveCurrFlasher(this);
						flasher_openFlasher(this, getRandomInt(0, this->flasher_cnt-1));
					}
				}
				// new flasher
				if (shift && (key == GLFW_KEY_A)) {
					if (this->flasher_idx >= 0) {
						flasher_saveCurrFlasher(this);
					}
					flasher_newFlasher(this);
					flasher_openFlasher(this, this->flasher_idx);
				}
				// delete flasher
				if (shift && (key == GLFW_KEY_D)) {
					if (this->flasher_cnt > 0) {
						flasher_deleteFlasher(this);
						if (this->flasher_idx >= 0) {
							flasher_openFlasher(this, this->flasher_idx);
						}
					}
				}
			}
		}
	}

	if (this->textbox_focus) {
		// prevent trailing '/' or 'I' character
		if (this->textbox_just_focus) {
			for (int i = 0; i < e->char_cnt; i++) {
				if (e->chars[i] == '/' || e->chars[i] == 'i' || e->chars[i] == 'I' || e->chars[i] == 't' || e->chars[i] == 'T' || e->chars[i] == 'a') {
					memmove(&e->chars[i], &e->chars[i + 1], (e->char_cnt - i - 1) * sizeof(e->chars[0]));
					e->char_cnt--;
					this->textbox_just_focus = false;
					break;
				}
			}
		}
		textbox_update(&this->textbox, ctx, e);
	}
}


void flasher_render(Flasher* this, Renderer* renderer) {
	// render label
	char flasher_label[16] = {0};
	snprintf(flasher_label, sizeof(flasher_label), "%d/%d", this->flasher_idx+1, this->flasher_cnt);
	int flasher_label_length = strlen(flasher_label);

	double char_size = 8.0 * 2.0;
	double cx = 1180, cy = 20;
	for (int i = flasher_label_length-1; i >= 0; i--) {
		cx -= char_size;
		renderer_drawChar(renderer, flasher_label[i], cx, cy, 2.0, 0x000000FF);
	}

	// render instruction
	const char* info_label = "(A)dd (R)andom (D)elete";
	int info_label_length = strlen(info_label);

	cx = 20; cy = 20;
	for (int i = 0; i < info_label_length; i++) {
		renderer_drawChar(renderer, info_label[i], cx, cy, 2.0, 0x000000FF);
		cx += char_size;
	}

	// render textbox
	if (this->flasher_idx >= 0) {
		textbox_render(&this->textbox, renderer);
	}

	// render status
	if (this->flasher_idx >= 0) {
		char status_label[128] = {0};
		if (this->textbox_focus) {
			int sel_len = 0;
			if (this->textbox.sel_start != -1) {
				sel_len = abs(this->textbox.cursor_idx - this->textbox.sel_start);
			}
			snprintf(status_label, sizeof(status_label), "(Unsaved) row:%d col:%d sel:%d len:%d", this->textbox.cursor_row, this->textbox.cursor_col, sel_len, this->textbox.length);
		}
		else {
			const char* file_name = this->flashers + this->stride * this->flasher_idx;
			char file_path[9+FLASHER_FILE_NAME_LENGTH+8+1];
			sprintf(file_path, "%s%s%s", FLASHER_FOLDER_PATH, this->flashers + this->stride * this->flasher_idx, FLASHER_FILE_EXTENSION);

			snprintf(status_label, sizeof(status_label), "Saved! ""%s"", %zu byte(s)", file_path, get_file_size(file_path));
		}

		int status_label_length = strlen(status_label);
		char_size = 8.0 * 2.0;
		cx = 1180; cy = this->textbox.y + this->textbox.h + 15.0;
		for (int i = status_label_length-1; i >= 0; i--) {
			cx -= char_size;
			renderer_drawChar(renderer, status_label[i], cx, cy, 2.0, 0x000000FF);
		}
	}
}
