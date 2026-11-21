#include "modules.h"


static int textbox_tabAdvance(int col) {
	return TEXTBOX_TAB_SIZE - (col % TEXTBOX_TAB_SIZE);
}


bool textbox_coordInTextbox(Textbox* this, double x, double y) {
	return x >= this->x && x <= this->x + this->w &&
	       y >= this->y && y <= this->y + this->h;
}


TextboxHistory textbox_createHistory(Textbox* this) {
	TextboxHistory history = {0};
	memcpy(history.buffer, this->buffer, TEXTBOX_MAX_BUFFER_LENGTH+1);
	history.cursor_idx = this->cursor_idx;
	history.length = this->length;
	return history;
}
 

void textbox_syncWrapBuffer(Textbox* this) {
	double char_size = 8.0 * this->font_scale;
	double max_x = this->x + this->w - this->padding;

	// copy buffer in wrap_buffer
	memcpy(this->wrap_buffer, this->buffer, this->length+1);

	double cx = this->x + this->padding;
	int col = 0;
	int last_space = -1;

	for (int i = 0; i < this->length; i++) {
		// newline
		if (this->wrap_buffer[i] == '\n') {
			cx = this->x + this->padding;
			col = 0;
			last_space = -1;
			continue;
		}
		
		// if space encountered, set last space
		if (this->wrap_buffer[i] == ' ' || this->wrap_buffer[i] == '\t') {
			last_space = i;
		}

		// check ending space
		int advance_cols = this->wrap_buffer[i] == '\t' ? textbox_tabAdvance(col) : 1;
		double advance_px = advance_cols * char_size;

		if (cx + advance_px > max_x && last_space != -1) {
			this->wrap_buffer[last_space] = '\n';
			i = last_space;
			cx = this->x + this->padding;
			col = 0;
			last_space = -1;
		} else {
			cx += advance_px;
			col += advance_cols;
		}
	}
}


void textbox_getCursorRowCol(Textbox* this, int cursor_idx, int* cursor_row, int* cursor_col) {
	double char_size = 8.0 * this->font_scale;

	// iterate
	int row = 0;
	int col = 0;
	for (int i = 0; i < cursor_idx; i++) {
		if (this->wrap_buffer[i] == '\n') {
			row++;
			col = 0;
		}
		else if (this->wrap_buffer[i] == '\t') {
			col += textbox_tabAdvance(col);
		}
		else col++;
	}

	// apply value
	*cursor_row = row;
	*cursor_col = col;
}


void textbox_pushHistory(Textbox* this) {
	// if idx is at max history
	if (this->history_idx >= TEXTBOX_MAX_HISTORY-1) {
		memmove(this->history, this->history + 1, (this->history_cnt-1)*sizeof(TextboxHistory));
		this->history[this->history_idx] = textbox_createHistory(this);
		return;
	}

	// if idx == cnt
	if (this->history_idx == this->history_cnt-1) {
		this->history_idx++;
		this->history_cnt++;
		this->history[this->history_idx] = textbox_createHistory(this);
		return;
	}

	// if idx == cnt
	if (this->history_idx < this->history_cnt-1) {
		this->history_idx++;
		this->history_cnt = this->history_idx+1;
		this->history[this->history_idx] = textbox_createHistory(this);
	}
}


void textbox_blinkCursor(Textbox* this) {
	double now_ms = getTimeNs() / 1000000.0;
	if (now_ms - this->last_blink_ms >= TEXTBOX_BLINK_INTERVAL) {
		this->cursor_display = !this->cursor_display;
		this->last_blink_ms = now_ms;
	}
}


void textbox_insertTextAtCursor(Textbox* this, int start_idx, const char* text) {
	// get length
	int length = strlen(text);
	if (this->length + length > TEXTBOX_MAX_BUFFER_LENGTH) {
		length = TEXTBOX_MAX_BUFFER_LENGTH - this->length;
		log_info("Textbox buffer overflow\n");
	}

	// move and copy in line of data
	if (length > 0) {
		memmove(this->buffer + start_idx + length, this->buffer + start_idx, this->length - start_idx);
		memcpy(this->buffer + start_idx, text, length);

		// update variable
		this->length += length;
		this->cursor_idx = (start_idx + length);
		this->buffer[this->length] = '\0';

		textbox_syncWrapBuffer(this);
		textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		this->cursor_max_col = this->cursor_col;
	}
}


void textbox_deleteBackText(Textbox* this, int start_idx, int magnitude) {
	if (start_idx <= 0 || start_idx > this->length || magnitude <= 0) {
		return;
	}
	// check index underflow
	if (start_idx - magnitude < 0) {
		magnitude = start_idx;
	}

	// delete text
	if (magnitude > 0) {
		memmove(this->buffer + start_idx - magnitude, this->buffer + start_idx, this->length - start_idx);
		this->length -= magnitude;
		this->cursor_idx = (start_idx - magnitude);
		this->buffer[this->length] = '\0';

		textbox_syncWrapBuffer(this);
		textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		this->cursor_max_col = this->cursor_col;
	}
}


void textbox_deleteFrontText(Textbox* this, int start_idx, int magnitude) {
	if (start_idx < 0 || start_idx >= this->length || magnitude <= 0) {
		return;
	}

	// clamp magnitude
	if (start_idx + magnitude > this->length) {
		magnitude = this->length - start_idx;
	}

	// shift character behind left
	memmove(this->buffer + start_idx, this->buffer + start_idx + magnitude, (this->length - (start_idx + magnitude)) + 1);
	this->length -= magnitude;
	this->cursor_idx = start_idx;

	textbox_syncWrapBuffer(this);
	textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
	this->cursor_max_col = this->cursor_col;
}


void textbox_navigateHorizontal(Textbox* this, int dir) {
	this->cursor_idx = clampInt(0, this->cursor_idx + dir, this->length);
	textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
	this->cursor_max_col = this->cursor_col;

	if (this->sel_start != -1 && this->cursor_idx == this->sel_start) {
		this->sel_start = -1;
	}
}


void textbox_navigateUp(Textbox* this) {
	// goto upper line
	while (this->cursor_idx > 0 && this->wrap_buffer[this->cursor_idx-1] != '\n') {
		this->cursor_idx--;
	}

	// if at index 0
	if (this->cursor_idx <= 0) {
		textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		return;
	}

	this->cursor_idx--;
	// count how much character in the line by going to start of the line
	while (this->cursor_idx > 0 && this->wrap_buffer[this->cursor_idx-1] != '\n') {
		this->cursor_idx--;
	}

	// col
	int col = 0;
	while (this->wrap_buffer[this->cursor_idx] != '\0' && this->wrap_buffer[this->cursor_idx] != '\n' && col < this->cursor_max_col) {
		int advance_cols = this->wrap_buffer[this->cursor_idx] == '\t' ? textbox_tabAdvance(col) : 1;
		if (col + advance_cols > this->cursor_max_col) {
			break;
		}
		col += advance_cols;
		this->cursor_idx++;
	}

	textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
	if (this->sel_start != -1 && this->cursor_idx == this->sel_start) {
		this->sel_start = -1;
	}
}


void textbox_navigateDown(Textbox* this) {
	// goto next line
	while (this->cursor_idx < this->length && this->wrap_buffer[this->cursor_idx] != '\n') {
		this->cursor_idx++;
	}

	// if the end of file return
	if (this->cursor_idx >= this->length) {
		textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		return;
	}

	this->cursor_idx++;
	// col
	int col = 0;
	while (this->wrap_buffer[this->cursor_idx] != '\0' && this->wrap_buffer[this->cursor_idx] != '\n' && col < this->cursor_max_col) {
		int advance_cols = this->wrap_buffer[this->cursor_idx] == '\t' ? textbox_tabAdvance(col) : 1;
		if (col + advance_cols > this->cursor_max_col) {
			break;
		}
		col += advance_cols;
		this->cursor_idx++;
	}
	textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
	if (this->sel_start != -1 && this->cursor_idx == this->sel_start) {
		this->sel_start = -1;
	}
}


int textbox_cursorIdxFromCoord(Textbox* this, double x, double y) {
	// set variable
	double char_size = 8.0 * this->font_scale;
	double line_height = this->line_space + char_size;

	int tar_row = (y - (this->y + this->padding)) / line_height;
	if (tar_row < 0) tar_row = 0;

	int row = 0, col = 0;
	double cx = this->x + this->padding;

	for (int i = 0; i < this->length; i++) {
		if (this->wrap_buffer[i] == '\n') {
			if (row == tar_row) {
				return i;
			}
			row++;
			cx = this->x + this->padding;
			col = 0;
			continue;
		}

		int advance_col = (this->wrap_buffer[i] == '\t') ? textbox_tabAdvance(col) : 1;
		double advance_px = advance_col * char_size;
		if (row == tar_row && x < cx + advance_px * 0.5) {
			return i;
		}

		cx += advance_px;
		col += advance_col;
	}

	return this->length;
}



Textbox textbox_create(int x, int y, int w, int h) {
	Textbox this = {0};
	this.buffer[0] = '\0';
	this.wrap_buffer[0] = '\0';
	this.length = 0;

	this.x = x;
	this.y = y;
	this.w = w;
	this.h = h;
	this.fg_color = 0x000000ff;
	this.bg_color = 0xffffffff;
	this.sel_fg_color = 0xffffffff;
	this.sel_bg_color = 0xb85e5eff;

	this.line_space = 10.0;
	this.font_scale = TEXTBOX_INITIAL_FONT_SCALE;
	this.padding = 10.0;

	this.cursor_idx = this.length;
	this.cursor_row = 0;
	this.cursor_max_col = 0;
	this.cursor_col = 0;
	this.sel_start = -1;

	this.last_blink_ms = (double) getTimeNs() / 1000000.0;
	this.cursor_display = false;

	this.mouse_down_prev = false;
	this.is_dragging = false;
	this.last_click_idx = -1;
	this.last_click_time_ms = (double)getTimeNs() / 1000000.0;
	this.click_cnt = 0;

	this.history = memory_alloc(TEXTBOX_MAX_HISTORY * sizeof(TextboxHistory));
	this.history_cnt = 0;
	this.history_idx = -1;

	return this;
}


void textbox_keyLeft(Textbox* this, bool shift, bool ctrl) {
	// check return to unselect mode
    if (this->sel_start != -1 && !shift) {
        this->cursor_idx = minInt(this->sel_start, this->cursor_idx);
        this->sel_start = -1;
        textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
        goto reset_blink;
    }

    if (this->cursor_idx <= 0) {
        goto reset_blink;
    }

	// start selection
    if (this->sel_start == -1 && shift) {
        this->sel_start = this->cursor_idx;
    }
    if (ctrl) {
        int cnt = 0;
        if (isalnum(this->buffer[this->cursor_idx - 1])) {
            while (this->cursor_idx - 1 - cnt >= 0 && isalnum(this->buffer[this->cursor_idx - 1 - cnt])) {
                cnt++;
            }
        } else if (is_tab_or_space(this->buffer[this->cursor_idx - 1])) {
            while (this->cursor_idx - 1 - cnt >= 0 && is_tab_or_space(this->buffer[this->cursor_idx - 1 - cnt])) {
                cnt++;
            }
        } else if (this->buffer[this->cursor_idx-1] == '\n') {
			cnt++;
		} else {
            while (this->cursor_idx - 1 - cnt >= 0 && !isalnum(this->buffer[this->cursor_idx-1-cnt]) && !is_tab_or_space(this->buffer[this->cursor_idx-1-cnt])) {
                cnt++;
            }
		}
        textbox_navigateHorizontal(this, -cnt);
    } else {
        textbox_navigateHorizontal(this, -1);
    }

reset_blink:
	this->cursor_display = true;
	this->last_blink_ms = (double)getTimeNs() / 1000000.0;
}


void textbox_keyRight(Textbox* this, bool shift, bool ctrl) {
	// chrcks mod key
	if (this->sel_start != -1 && !shift) {
		this->cursor_idx = maxInt(this->sel_start, this->cursor_idx);
		this->sel_start = -1;
		textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		goto reset_blink;
	}

	if (this->cursor_idx >= this->length) {
		goto reset_blink;
	}

	// start selection
	if (this->sel_start == -1 && shift) {
		this->sel_start = this->cursor_idx;
	}
	if (ctrl) {
		int cnt = 0;
		if (isalnum(this->buffer[this->cursor_idx])) {
			while (this->cursor_idx + cnt < this->length && isalnum(this->buffer[this->cursor_idx + cnt])) {
				cnt++;
			}
		} else if (is_tab_or_space(this->buffer[this->cursor_idx])) {
			while (this->cursor_idx + cnt < this->length && is_tab_or_space(this->buffer[this->cursor_idx + cnt])) {
				cnt++;
			}
		} else if (this->buffer[this->cursor_idx] == '\n') {
			cnt++;
		} else {
			while (this->cursor_idx + cnt < this->length && !isalnum(this->buffer[this->cursor_idx+cnt]) && !is_tab_or_space(this->buffer[this->cursor_idx+cnt])) {
				cnt++;
			}
		}
		textbox_navigateHorizontal(this, cnt);
	}
	else {
		textbox_navigateHorizontal(this, 1);
	}

reset_blink:
	this->cursor_display = true;
	this->last_blink_ms = (double)getTimeNs() / 1000000.0;
}


void textbox_keyUp(Textbox* this, bool shift, bool ctrl) {
	// checks mod key
	if (this->sel_start != -1 && !shift) {
		this->cursor_idx = minInt(this->sel_start, this->cursor_idx);
		this->sel_start = -1;
		textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		goto reset_blink;
	}

    if (this->cursor_idx <= 0) {
        goto reset_blink;
    }

	// start selection
	if (this->sel_start == -1 && shift) {
		this->sel_start = this->cursor_idx;
	}
	textbox_navigateUp(this);

reset_blink:
	this->cursor_display = true;
	this->last_blink_ms = (double)getTimeNs() / 1000000.0;
}


void textbox_keyDown(Textbox* this, bool shift, bool ctrl) {
	// checks mod key
	if (this->sel_start != -1 && !shift) {
		this->cursor_idx = maxInt(this->sel_start, this->cursor_idx);
		this->sel_start = -1;
		textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		goto reset_blink;
	}

    if (this->cursor_idx >= this->length) {
        goto reset_blink;
    }

	// start selection
	if (this->sel_start == -1 && shift) {
		this->sel_start = this->cursor_idx;
	}
	textbox_navigateDown(this);

reset_blink:
	this->cursor_display = true;
	this->last_blink_ms = (double)getTimeNs() / 1000000.0;
}


void textbox_backspace(Textbox* this, bool shift, bool ctrl) {
	// if selected, just delete the selected text
	if (this->sel_start != -1) {
		textbox_deleteFrontText(this, minInt(this->sel_start, this->cursor_idx), abs(this->sel_start - this->cursor_idx));
		this->sel_start = -1;
		goto push_history;
	}

	// if cursor at 0
	if (this->cursor_idx <= 0) {
		goto reset_blink;
	}

	// main logic
	if (ctrl) {
		int cnt = 0;
        if (isalnum(this->buffer[this->cursor_idx - 1])) {
            while (this->cursor_idx - 1 - cnt >= 0 && isalnum(this->buffer[this->cursor_idx - 1 - cnt])) {
                cnt++;
            }
        } else if (is_tab_or_space(this->buffer[this->cursor_idx - 1])) {
            while (this->cursor_idx - 1 - cnt >= 0 && is_tab_or_space(this->buffer[this->cursor_idx - 1 - cnt])) {
                cnt++;
            }
        } else if (this->buffer[this->cursor_idx-1] == '\n') {
			cnt++;
		} else {
            while (this->cursor_idx - 1 - cnt >= 0 && !isalnum(this->buffer[this->cursor_idx-1-cnt]) && !is_tab_or_space(this->buffer[this->cursor_idx-1-cnt])) {
                cnt++;
            }
		}
		textbox_deleteBackText(this, this->cursor_idx, cnt);
	}
	else textbox_deleteBackText(this, this->cursor_idx, 1);

push_history:
	textbox_pushHistory(this);

reset_blink:
	this->cursor_display = true;
	this->last_blink_ms = (double)getTimeNs() / 1000000.0;
}


void textbox_delete(Textbox* this, bool shift, bool ctrl) {
	// delete the selected text
	if (this->sel_start != -1) {
		textbox_deleteFrontText(this, minInt(this->sel_start, this->cursor_idx), abs(this->sel_start - this->cursor_idx));
		this->sel_start = -1;
		goto push_history;
	}

	// nothing to delete
	if (this->cursor_idx >= this->length) {
		goto reset_blink;
	}

	// main logic
	if (ctrl) {
		int cnt = 0;
		if (isalnum(this->buffer[this->cursor_idx])) {
			while (this->cursor_idx + cnt < this->length && isalnum(this->buffer[this->cursor_idx + cnt])) {
				cnt++;
			}
		} else if (is_tab_or_space(this->buffer[this->cursor_idx])) {
			while (this->cursor_idx + cnt < this->length && is_tab_or_space(this->buffer[this->cursor_idx + cnt])) {
				cnt++;
			}
		} else if (this->buffer[this->cursor_idx] == '\n') {
			cnt++;
		} else {
			while (this->cursor_idx + cnt < this->length && !isalnum(this->buffer[this->cursor_idx + cnt]) && !is_tab_or_space(this->buffer[this->cursor_idx + cnt])) {
				cnt++;
			}
		}
		textbox_deleteFrontText(this, this->cursor_idx, cnt);
	} else {
		textbox_deleteFrontText(this, this->cursor_idx, 1);
	}

push_history:
	textbox_pushHistory(this);

reset_blink:
	this->cursor_display = true;
	this->last_blink_ms = (double)getTimeNs() / 1000000.0;
}


void textbox_typeText(Textbox* this, Event* e) {
	if (e->char_cnt > 0) {
		char text[EVENT_MAX_FRAME_EVENT+1] = {0};
		int cnt = 0;
		for (int i = 0; i < e->char_cnt; i++) {
			if (e->chars[i] >= 32 && e->chars[i] < 127) {
				text[cnt] = (char)e->chars[i];
				cnt++;
			}
		}
		text[cnt] = '\0';

		if (this->sel_start != -1) {
			textbox_deleteBackText(this, maxInt(this->sel_start, this->cursor_idx), abs(this->sel_start - this->cursor_idx));
			this->sel_start = -1;
			textbox_insertTextAtCursor(this, this->cursor_idx, text);
		}
		else {
			textbox_insertTextAtCursor(this, this->cursor_idx, text);
		}

		textbox_pushHistory(this);
	}
}


void textbox_history(Textbox* this, bool shift) {
	// detect if changed undo or redo
	bool changed = false;
	if (shift) {
		if (this->history_idx < this->history_cnt - 1) {
			this->history_idx++;
			changed = true;
		}
	}
	else {
		if (this->history_idx > 0) {
			this->history_idx--;
			changed = true;
		}
	}

	// copy history to current textbox
	if (changed) {
		TextboxHistory* history = &this->history[this->history_idx];
		memcpy(this->buffer, history->buffer, TEXTBOX_MAX_BUFFER_LENGTH+1);
		this->length = history->length;
		this->cursor_idx = history->cursor_idx;

		this->sel_start = -1;

		textbox_syncWrapBuffer(this);
		textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		this->cursor_max_col = this->cursor_col;
	}

	this->cursor_display = true;
	this->last_blink_ms = (double)getTimeNs() / 1000000.0;
}


void textbox_tab(Textbox* this, bool shift) {
	if (this->sel_start != -1 && this->sel_start != this->cursor_idx) {
		int start = minInt(this->sel_start, this->cursor_idx);
		int end   = maxInt(this->sel_start, this->cursor_idx);

		// find start of the first selected line
		int line_start = start;
		while (line_start > 0 && this->buffer[line_start - 1] != '\n') {
			line_start--;
		}

		// if selection ends at column 0 of a line (right after '\n'), don't include that trailing line
		if (end > start && this->buffer[end - 1] == '\n') {
			end--;
		}

		int idx = line_start;
		while (idx <= end) {
			textbox_insertTextAtCursor(this, idx, "\t");
			if (idx <= start) {
				start++;
			}
			end++;
			idx++; // skip the newly inserted \t

			// advance to start of the next line
			while (idx <= end && this->buffer[idx] != '\n') {
				idx++;
			}
			idx++; // step past \n
		}

		// restore selection bounds
		if (this->cursor_idx > this->sel_start) {
			this->sel_start  = start;
			this->cursor_idx = end;
		} else {
			this->sel_start  = end;
			this->cursor_idx = start;
		}
	} else {
		// Single cursor (no selection)
		textbox_insertTextAtCursor(this, this->cursor_idx, "\t");
	
	}

	textbox_pushHistory(this);
	this->cursor_display = true;
	this->last_blink_ms = (double)getTimeNs() / 1000000.0;
}

void textbox_copy(Textbox* this, Context* ctx) {
	if (this->sel_start == -1) return;

	int start = minInt(this->cursor_idx, this->sel_start);
	int stop = maxInt(this->cursor_idx, this->sel_start);
	int length = stop - start;

	char temp[TEXTBOX_MAX_BUFFER_LENGTH + 1];
	memcpy(temp, this->buffer + start, length);
	temp[length] = '\0';

	glfwSetClipboardString(ctx->glfw_window, temp);
}


static void textbox_cut(Textbox* this, Context* ctx) {
	if (this->sel_start == -1) return;

	textbox_copy(this, ctx);
	textbox_delete(this, false, false);
	textbox_pushHistory(this);
}


void textbox_update(Textbox* this, Context* ctx, Event* e) {
	// keyboard state
	bool shift = (glfwGetKey(ctx->glfw_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(ctx->glfw_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
	bool ctrl = (glfwGetKey(ctx->glfw_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(ctx->glfw_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

	// mouse state
	double mouse_x, mouse_y;
	glfwGetCursorPos(ctx->glfw_window, &mouse_x, &mouse_y);

	bool mouse_down = (glfwGetMouseButton(ctx->glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
	bool mouse_just_down = mouse_down && !this->mouse_down_prev;
	bool mouse_just_up = !mouse_down && this->mouse_down_prev;
	this->mouse_down_prev = mouse_down;

	bool mouse_in_textbox = textbox_coordInTextbox(this, mouse_x, mouse_y);
	int click_idx = textbox_cursorIdxFromCoord(this, mouse_x, mouse_y);

	if (mouse_in_textbox || this->is_dragging) glfwSetCursor(ctx->glfw_window, glfwCreateStandardCursor(GLFW_IBEAM_CURSOR));
	else glfwSetCursor(ctx->glfw_window, NULL);

	// type text
	textbox_typeText(this, e);

	// mouse event
	if (mouse_just_down && mouse_in_textbox) {
		// check click count
		double now_ms = (double)getTimeNs()/1000000.0;
		if (!shift && (now_ms - this->last_click_time_ms <= TEXTBOX_CLICK_THRESHOLD_MS) && (click_idx == this->last_click_idx)) {
			this->click_cnt = (this->click_cnt % 3) + 1;
		} else {
			this->click_cnt = 1;
		}
		this->last_click_time_ms = now_ms;
		this->last_click_idx = click_idx;

		// if (this->click_count == 3) {
		// 	textbox_selectParagraph(this, click_idx);
		// 	this->is_dragging = false;
		// } else if (this->click_count == 2) {
		// 	textbox_selectWord(this, click_idx);
		// 	this->is_dragging = false;
		if (shift) {
			if (this->sel_start == -1) {
				this->sel_start = this->cursor_idx;
			}
			this->cursor_idx = click_idx;
			this->is_dragging = true;
			textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		} else {
			this->cursor_idx = click_idx;
			this->sel_start = click_idx;
			this->is_dragging = true;
			textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		}
	}
	if (this->is_dragging && mouse_down) {
		if (click_idx != this->cursor_idx) {
			this->cursor_idx = click_idx;
			textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
		}
	}
	if (mouse_just_up && this->is_dragging) {
		this->is_dragging = false;
		if (this->sel_start == this->cursor_idx) {
			this->sel_start = -1;
		}
	}

	// check incoming key event
	for (int i = 0; i < e->key_cnt; i++) {
		if (e->keys[i].action == GLFW_PRESS || e->keys[i].action == GLFW_REPEAT) {
			// retrieve keys
			int key = e->keys[i].key;

			// increase font scale
			if (ctrl && key == GLFW_KEY_EQUAL) {
				this->font_scale = clampDouble(TEXTBOX_MIN_FONT_SCALE, this->font_scale + 1.0, INT_MAX);
				textbox_syncWrapBuffer(this);
				textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
				this->cursor_max_col = this->cursor_col;

				this->cursor_display = true;
				this->last_blink_ms = (double)getTimeNs() / 1000000.0;
			}
			// decrease font scale
			if (ctrl && key == GLFW_KEY_MINUS) {
				this->font_scale = clampDouble(TEXTBOX_MIN_FONT_SCALE, this->font_scale - 1.0, INT_MAX);
				textbox_syncWrapBuffer(this);
				textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
				this->cursor_max_col = this->cursor_col;

				this->cursor_display = true;
				this->last_blink_ms = (double)getTimeNs() / 1000000.0;
			}
			// newline
			if (key == GLFW_KEY_ENTER) {
				if (this->sel_start != -1) {
					textbox_deleteFrontText(this, minInt(this->sel_start, this->cursor_idx), abs(this->sel_start - this->cursor_idx));
					this->sel_start = -1;
				}
				textbox_insertTextAtCursor(this, this->cursor_idx, "\n");
				textbox_pushHistory(this);
				this->cursor_display = true;
				this->last_blink_ms = (double)getTimeNs() / 1000000.0;
			}
			if (key == GLFW_KEY_TAB) {
				textbox_tab(this, shift);
			}
			// cursor left
			if (key == GLFW_KEY_LEFT) {
				textbox_keyLeft(this, shift, ctrl);
			}
			// cursor right
			if (key == GLFW_KEY_RIGHT) {
				textbox_keyRight(this, shift, ctrl);
			}
			// cursor up
			if (key == GLFW_KEY_UP) {
				textbox_keyUp(this, shift, ctrl);
			}
			// cursor down
			if (key == GLFW_KEY_DOWN) {
				textbox_keyDown(this, shift, ctrl);
			}
			// backspace
			if (key == GLFW_KEY_BACKSPACE) {
				textbox_backspace(this, shift, ctrl);
			}
			// delete
			if (key == GLFW_KEY_DELETE) {
				textbox_delete(this, shift, ctrl);
			}
			// select all
			if (ctrl && (key == GLFW_KEY_A)) {
				this->sel_start = 0;
				this->cursor_idx = this->length;
				textbox_getCursorRowCol(this, this->cursor_idx, &this->cursor_row, &this->cursor_col);
				this->cursor_display = true;
				this->last_blink_ms = (double)getTimeNs() / 1000000.0;
			}
			// history
			if (ctrl && (key == GLFW_KEY_Z)) {
				textbox_history(this, shift);
			}
			// copy
			if (ctrl && (key == GLFW_KEY_C)) {
				textbox_copy(this, ctx);
			}
			// cut
			if (ctrl && (key == GLFW_KEY_X)) {
				textbox_cut(this, ctx);
			}
			// paste
			if (ctrl && (key == GLFW_KEY_V)) {
				const char* clip = glfwGetClipboardString(ctx->glfw_window);
				textbox_insertTextAtCursor(this, this->cursor_idx, clip);
				textbox_pushHistory(this);
				this->cursor_display = true;
				this->last_blink_ms = (double)getTimeNs() / 1000000.0;
			}
		}
	}

	textbox_blinkCursor(this);
}


void textbox_render(Textbox* this, Renderer* renderer) {
	// render background
	renderer_fillRect(renderer, this->x, this->y, this->w, this->h, this->bg_color);

	double char_size = 8.0 * this->font_scale;
	double cx = this->x + this->padding, cy = this->y + this->padding;
	int sel_min_idx = minInt(this->sel_start, this->cursor_idx);
	int sel_max_idx = maxInt(this->sel_start, this->cursor_idx);

	// render text
	int col = 0;
	for (int i = 0; i < this->length; i++) {
		if (this->wrap_buffer[i] == '\n') {
			if (this->sel_start != -1 && sel_min_idx <= i && i < sel_max_idx) {
				renderer_fillRect(renderer, cx, cy-3.0, (this->x + this->w - this->padding - cx), char_size+8.0, this->sel_bg_color);
			}
			col = 0;
			cx = this->x + this->padding;
			cy += char_size + this->line_space;
		}
		else {
			int advance_cols = this->wrap_buffer[i] == '\t' ? textbox_tabAdvance(col) : 1;
			int advance_px = advance_cols * char_size;
			if (this->sel_start != -1 && sel_min_idx <= i && i < sel_max_idx) {
				renderer_fillRect(renderer, cx, cy-3.0, advance_px, char_size+8.0, this->sel_bg_color);
				if (this->wrap_buffer[i] != '\t') {
					renderer_drawChar(renderer, this->wrap_buffer[i], cx, cy, this->font_scale, this->sel_fg_color);
				}
			}
			else {
				if (this->wrap_buffer[i] != '\t') {
					renderer_drawChar(renderer, this->wrap_buffer[i], cx, cy, this->font_scale, this->fg_color);
				}
			}
			col += advance_cols;
			cx += advance_px;
		}
	}

	// render cursor
	cx = this->x + this->padding + this->cursor_col * char_size;
	cy = this->y + this->padding + this->cursor_row * (char_size + this->line_space);
	if (this->cursor_display) {
		renderer_fillRect(renderer, cx, cy, 4.0, char_size, this->fg_color);
	}
}
