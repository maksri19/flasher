// #include "Modules.h"
// #define MAX_STRING_LENGTH (1024)
// #define INITIAL_FONT_SCALE (2.0)
// #define MIN_FONT_SCALE (0.4)
// #define MAX_UNDO_HISTORY (32)
// #define TAB_SIZE (8)
//
//
// static int tab_advance(int col) {
// 	return TAB_SIZE - (col % TAB_SIZE);
// }
//
//
// static bool is_space_or_tab(char c) {
// 	return c == ' ' || c == '\t';
// }
//
//
// static bool is_word_char(char c) {
// 	return (c >= 'a' && c <= 'z') ||
// 	       (c >= 'A' && c <= 'Z') ||
// 	       (c >= '0' && c <= '9') ||
// 	       c == '_';
// }
//
//
// static const char* sync_wrap_buffer(const Textbox* this) {
// 	double char_size = 8.0 * this->font_scale;
// 	double max_x = this->x + this->w - this->padding;
//
// 	memcpy(this->wrap_buffer, this->buffer, this->len);
// 	out_buffer[this->len] = '\0';
//
// 	double cx = this->x + this->padding;
// 	int col = 0;
// 	int last_space = -1;
//
// 	for (int i = 0; i < this->len; i++) {
// 		if (this->wrap_buffer[i] == '\n') {
// 			cx = this->x + this->padding;
// 			col = 0;
// 			last_space = -1;
// 			continue;
// 		}
// 		if (is_space_or_tab(this->wrap_buffer[i])) {
// 			last_space = i;
// 		}
//
// 		int advance_cols = (this->wrap_buffer[i] == '\t') ? tab_advance(col) : 1;
// 		double advance_px = advance_cols * char_size;
//
// 		if (cx + advance_px > max_x && last_space != -1) {
// 			this->wrap_buffer[last_space] = '\n';
// 			i = last_space;
// 			cx = this->x + padding;
// 			col = 0;
// 			last_space = -1;
// 		} else {
// 			cx += advance_px;
// 			col += advance_cols;
// 		}
// 	}
// }
//
//
// static void sync_preferred_col(Textbox* this) {
// 	int col = 0;
// 	for (int i = 0; i < this->cursor_pos; i++) {
// 		if (this->wrap_buffer[i] == '\n') col = 0;
// 		else if (this->wrap_buffer[i] == '\t') col += textbox_tabAdvance(col);
// 		else col += 1;
// 	}
// 	this->preferred_col = col;
// }
//
//
// static void textbox_saveSnapshot(TextboxState* dst, const Textbox* src) {
// 	memcpy(dst->buffer, src->buffer, src->len + 1);
// 	dst->len = src->len;
// 	dst->cursor_pos = src->cursor_pos;
// 	dst->select_start = src->select_start;
// }
//
//
// static void textbox_restoreSnapshot(Textbox* dst, const TextboxState* src) {
// 	memcpy(dst->buffer, src->buffer, src->len + 1);
// 	dst->len = src->len;
// 	dst->cursor_pos = src->cursor_pos;
// 	dst->select_start = src->select_start;
// 	sync_preferred_col(dst);
// }
//
//
// static void textbox_pushHistory(Textbox* this) {
// 	if (this->history_count > 0) {
// 		TextboxState* cur = &this->history[this->history_index];
// 		if (cur->len == this->len && memcmp(cur->buffer, this->buffer, this->len) == 0) {
// 			return;
// 		}
// 	}
//
// 	// If branching from before the saved state, the saved state is no longer in history
// 	if (this->saved_history_index > this->history_index) {
// 		this->saved_history_index = -1;
// 	}
//
// 	// Discard any redo states ahead of the current index
// 	this->history_count = this->history_index + 1;
//
// 	// If full, shift older entries left to make room at the top
// 	if (this->history_count >= MAX_UNDO_HISTORY) {
// 		memmove(&this->history[0], &this->history[1], sizeof(TextboxState) * (MAX_UNDO_HISTORY - 1));
// 		this->history_count = MAX_UNDO_HISTORY - 1;
// 		this->history_index = this->history_count - 1;
// 		if (this->saved_history_index >= 0) {
// 			this->saved_history_index--; // shifts to -1 if evicted off the bottom
// 		}
// 	}
//
// 	this->history_index++;
// 	this->history_count = this->history_index + 1;
// 	textbox_saveSnapshot(&this->history[this->history_index], this);
//
// 	this->is_dirty = (this->history_index != this->saved_history_index);
// }
//
// static void textbox_undo(Textbox* this) {
// 	if (this->history_index <= 0) return;
// 	this->history_index--;
// 	textbox_restoreSnapshot(this, &this->history[this->history_index]);
// 	this->is_dirty = (this->history_index != this->saved_history_index);
// }
//
// static void textbox_redo(Textbox* this) {
// 	if (this->history_index + 1 >= this->history_count) return;
// 	this->history_index++;
// 	textbox_restoreSnapshot(this, &this->history[this->history_index]);
// 	this->is_dirty = (this->history_index != this->saved_history_index);
// }
//
// static bool textbox_hasSelection(const Textbox* this) {
// 	return this->select_start != -1 && this->select_start != this->cursor_pos;
// }
//
//
// static void textbox_getSelectionRange(const Textbox* this, int* out_start, int* out_end) {
// 	if (this->select_start < this->cursor_pos) {
// 		*out_start = this->select_start;
// 		*out_end = this->cursor_pos;
// 	} else {
// 		*out_start = this->cursor_pos;
// 		*out_end = this->select_start;
// 	}
// }
//
//
// int textbox_getSelectionLength(const Textbox* this) {
// 	if (!textbox_hasSelection(this)) {
// 		return 0;
// 	}
//
// 	int start, end;
// 	textbox_getSelectionRange(this, &start, &end);
// 	return end - start;
// }
//
//
// static void textbox_deleteSelection(Textbox* this) {
// 	if (!textbox_hasSelection(this)) {
// 		this->select_start = -1;
// 		return;
// 	}
//
// 	int start, end;
// 	textbox_getSelectionRange(this, &start, &end);
// 	int count = end - start;
//
// 	memmove(this->buffer + start, this->buffer + end, this->len - end + 1);
// 	this->len -= count;
// 	this->cursor_pos = start;
// 	this->select_start = -1;
// 	sync_preferred_col(this);
// }
//
//
// static void append(Textbox* this, const char* text) {
// 	int len = strlen(text);
// 	int sel_len = 0;
// 	if (textbox_hasSelection(this)) {
// 		int start, end;
// 		textbox_getSelectionRange(this, &start, &end);
// 		sel_len = end - start;
// 	}
//
// 	if (this->len - sel_len + len > MAX_STRING_LENGTH) {
// 		log_info("Buffer already max\n");
// 		return;
// 	}
//
// 	if (textbox_hasSelection(this)) {
// 		textbox_deleteSelection(this);
// 	} else {
// 		this->select_start = -1;
// 	}
//
// 	memmove(this->buffer + this->cursor_pos + len, this->buffer + this->cursor_pos, this->len - this->cursor_pos + 1);
// 	memcpy(this->buffer + this->cursor_pos, text, len);
//
// 	this->cursor_pos += len;
// 	this->len += len;
// 	sync_wrap_buffer(this);
// 	sync_preferred_col(this);
// 	sync_history(this);
// }
//
//
// static int textbox_adjustIndex(int idx, int edit_pos, int delta) {
// 	if (delta > 0) {
// 		return (idx >= edit_pos) ? (idx + delta) : idx;
// 	} else {
// 		int removed = -delta;
// 		if (idx <= edit_pos) return idx;
// 		if (idx <= edit_pos + removed) return edit_pos;
// 		return idx - removed;
// 	}
// }
//
//
// static void textbox_indentBlock(Textbox* this) {
// 	int sel_start, sel_end;
// 	textbox_getSelectionRange(this, &sel_start, &sel_end);
//
// 	int scan_end = sel_end;
// 	if (scan_end > sel_start && this->buffer[scan_end - 1] == '\n') {
// 		scan_end--;
// 	}
//
// 	int line_start = sel_start;
// 	while (line_start > 0 && this->buffer[line_start - 1] != '\n') {
// 		line_start--;
// 	}
//
// 	int line_count = 1;
// 	for (int i = line_start; i < scan_end; i++) {
// 		if (this->buffer[i] == '\n') line_count++;
// 	}
//
// 	if (this->len + line_count > MAX_STRING_LENGTH) {
// 		log_info("Buffer already max\n");
// 		return;
// 	}
//
// 	int pos = line_start;
// 	for (int l = 0; l < line_count; l++) {
// 		memmove(this->buffer + pos + 1, this->buffer + pos, this->len - pos + 1);
// 		this->buffer[pos] = '\t';
// 		this->len += 1;
//
// 		this->cursor_pos = textbox_adjustIndex(this->cursor_pos, pos, 1);
// 		this->select_start = textbox_adjustIndex(this->select_start, pos, 1);
// 		scan_end += 1;
//
// 		pos += 1;
// 		while (pos < this->len && this->buffer[pos] != '\n') pos++;
// 		if (pos < this->len && this->buffer[pos] == '\n') pos++;
// 	}
//
// 	sync_preferred_col(this);
// 	textbox_pushHistory(this);
// }
//
//
// static void textbox_unindentRange(Textbox* this, int range_start, int range_end) {
// 	int scan_end = range_end;
// 	if (scan_end > range_start && this->buffer[scan_end - 1] == '\n') {
// 		scan_end--;
// 	}
//
// 	int line_start = range_start;
// 	while (line_start > 0 && this->buffer[line_start - 1] != '\n') {
// 		line_start--;
// 	}
//
// 	int pos = line_start;
// 	bool changed = false;
//
// 	while (pos <= scan_end && pos <= this->len) {
// 		int remove_count = 0;
// 		if (pos < this->len && this->buffer[pos] == '\t') {
// 			remove_count = 1;
// 		} else {
// 			while (remove_count < TAB_SIZE && (pos + remove_count) < this->len && this->buffer[pos + remove_count] == ' ') {
// 				remove_count++;
// 			}
// 		}
//
// 		if (remove_count > 0) {
// 			memmove(this->buffer + pos, this->buffer + pos + remove_count, this->len - (pos + remove_count) + 1);
// 			this->len -= remove_count;
//
// 			this->cursor_pos = textbox_adjustIndex(this->cursor_pos, pos, -remove_count);
// 			if (this->select_start != -1) {
// 				this->select_start = textbox_adjustIndex(this->select_start, pos, -remove_count);
// 			}
// 			scan_end -= remove_count;
// 			changed = true;
// 		}
//
// 		while (pos < this->len && this->buffer[pos] != '\n') pos++;
// 		if (pos < this->len && this->buffer[pos] == '\n') pos++;
// 		else break;
// 	}
//
// 	if (changed) {
// 		sync_preferred_col(this);
// 		textbox_pushHistory(this);
// 	}
// }
//
//
// static void textbox_tab(Textbox* this, bool shift) {
// 	if (shift) {
// 		if (textbox_hasSelection(this)) {
// 			int start, end;
// 			textbox_getSelectionRange(this, &start, &end);
// 			textbox_unindentRange(this, start, end);
// 		} else {
// 			textbox_unindentRange(this, this->cursor_pos, this->cursor_pos);
// 		}
// 		return;
// 	}
//
// 	if (textbox_hasSelection(this)) {
// 		textbox_indentBlock(this);
// 		return;
// 	}
//
// 	textbox_append(this, "\t");
// }
//
//
// static void textbox_clear(Textbox* this) {
// 	this->buffer[0] = '\0';
// 	this->len = 0;
// 	this->cursor_pos = 0;
// 	this->select_start = -1;
// 	this->preferred_col = 0;
// 	textbox_pushHistory(this);
// }
//
//
// static void textbox_backspace(Textbox* this) {
// 	if (textbox_hasSelection(this)) {
// 		textbox_deleteSelection(this);
// 		textbox_pushHistory(this);
// 		return;
// 	}
// 	this->select_start = -1;
//
// 	if (this->cursor_pos <= 0) return;
//
// 	memmove(
// 		this->buffer + this->cursor_pos - 1,
// 		this->buffer + this->cursor_pos,
// 		this->len - this->cursor_pos + 1
// 	);
// 	this->cursor_pos--;
// 	this->len--;
// 	sync_preferred_col(this);
// 	textbox_pushHistory(this);
// }
//
//
// static void textbox_cbackspace(Textbox* this) {
// 	if (textbox_hasSelection(this)) {
// 		textbox_deleteSelection(this);
// 		textbox_pushHistory(this);
// 		return;
// 	}
// 	this->select_start = -1;
//
// 	if (this->cursor_pos <= 0) return;
//
// 	int target = this->cursor_pos;
// 	if (is_space_or_tab(this->buffer[target - 1])) {
// 		while (target > 0 && is_space_or_tab(this->buffer[target - 1])) target--;
// 	} else {
// 		while (target > 0 && !is_space_or_tab(this->buffer[target - 1]) && this->buffer[target - 1] != '\n') target--;
// 		if (target == this->cursor_pos && this->buffer[target - 1] == '\n') target--;
// 	}
//
// 	int deleted = this->cursor_pos - target;
// 	memmove(this->buffer + target, this->buffer + this->cursor_pos, this->len - this->cursor_pos + 1);
// 	this->cursor_pos = target;
// 	this->len -= deleted;
// 	sync_preferred_col(this);
// 	textbox_pushHistory(this);
// }
//
//
// static void textbox_delete(Textbox* this) {
// 	if (textbox_hasSelection(this)) {
// 		textbox_deleteSelection(this);
// 		textbox_pushHistory(this);
// 		return;
// 	}
// 	this->select_start = -1;
//
// 	if (this->cursor_pos >= this->len) return;
//
// 	memmove(
// 		this->buffer + this->cursor_pos,
// 		this->buffer + this->cursor_pos + 1,
// 		this->len - this->cursor_pos
// 	);
// 	this->len--;
// 	sync_preferred_col(this);
// 	textbox_pushHistory(this);
// }
//
//
// static void textbox_left(Textbox* this) {
// 	if (this->cursor_pos > 0) this->cursor_pos--;
// 	sync_preferred_col(this);
// }
//
//
// static void textbox_cleft(Textbox* this) {
// 	if (this->cursor_pos <= 0) return;
// 	if (is_space_or_tab(this->buffer[this->cursor_pos - 1])) {
// 		while (this->cursor_pos > 0 && is_space_or_tab(this->buffer[this->cursor_pos - 1])) this->cursor_pos--;
// 	} else {
// 		while (this->cursor_pos > 0 && !is_space_or_tab(this->buffer[this->cursor_pos - 1]) && this->buffer[this->cursor_pos - 1] != '\n') this->cursor_pos--;
// 		if (this->cursor_pos > 0 && this->buffer[this->cursor_pos - 1] == '\n') this->cursor_pos--;
// 	}
// 	sync_preferred_col(this);
// }
//
//
// static void textbox_right(Textbox* this) {
// 	if (this->cursor_pos < this->len) this->cursor_pos++;
// 	sync_preferred_col(this);
// }
//
//
// static void textbox_cright(Textbox* this) {
// 	if (this->cursor_pos >= this->len) return;
// 	if (is_space_or_tab(this->buffer[this->cursor_pos])) {
// 		while (this->cursor_pos < this->len && is_space_or_tab(this->buffer[this->cursor_pos])) this->cursor_pos++;
// 	} else {
// 		while (this->cursor_pos < this->len && !is_space_or_tab(this->buffer[this->cursor_pos]) && this->buffer[this->cursor_pos] != '\n') this->cursor_pos++;
// 		if (this->cursor_pos < this->len && this->buffer[this->cursor_pos] == '\n') this->cursor_pos++;
// 	}
// 	sync_preferred_col(this);
// }
//
//
// static void textbox_moveVertical(Textbox* this, int delta_line) {
// 	char wrapped[MAX_STRING_LENGTH + 1];
// 	textbox_wrap(this, wrapped);
//
// 	int lines[MAX_STRING_LENGTH + 1];
// 	int cols[MAX_STRING_LENGTH + 1];
//
// 	int line = 0, col = 0;
// 	for (int i = 0; i < this->len; i++) {
// 		lines[i] = line;
// 		cols[i] = col;
// 		if (wrapped[i] == '\n') {
// 			line++;
// 			col = 0;
// 		} else if (wrapped[i] == '\t') {
// 			col += textbox_tabAdvance(col);
// 		} else {
// 			col++;
// 		}
// 	}
// 	lines[this->len] = line;
// 	cols[this->len] = col;
//
// 	int total_lines = line + 1;
// 	int target_line = lines[this->cursor_pos] + delta_line;
//
// 	if (target_line < 0) {
// 		this->cursor_pos = 0;
// 		this->preferred_col = 0;
// 		return;
// 	}
// 	if (target_line >= total_lines) {
// 		this->cursor_pos = this->len;
// 		this->preferred_col = cols[this->len];
// 		return;
// 	}
//
// 	int best_pos = -1;
// 	for (int i = 0; i <= this->len; i++) {
// 		if (lines[i] == target_line) {
// 			best_pos = i;
// 			if (cols[i] >= this->preferred_col) {
// 				break;
// 			}
// 		}
// 	}
//
// 	if (best_pos != -1) {
// 		this->cursor_pos = best_pos;
// 	}
// }
//
//
// static void textbox_up(Textbox* this) {
// 	textbox_moveVertical(this, -1);
// }
//
//
// static void textbox_down(Textbox* this) {
// 	textbox_moveVertical(this, +1);
// }
//
//
// static void textbox_selectAll(Textbox* this) {
// 	if (this->len == 0) return;
// 	this->select_start = 0;
// 	this->cursor_pos = this->len;
// 	sync_preferred_col(this);
// }
//
//
// static void textbox_beginNav(Textbox* this, bool shift) {
// 	if (shift) {
// 		if (this->select_start == -1) {
// 			this->select_start = this->cursor_pos;
// 		}
// 	} else {
// 		this->select_start = -1;
// 	}
// }
//
//
// static void textbox_copy(Textbox* this, Context* context) {
// 	if (!textbox_hasSelection(this)) return;
//
// 	int start, end;
// 	textbox_getSelectionRange(this, &start, &end);
// 	int count = end - start;
//
// 	char temp[MAX_STRING_LENGTH + 1];
// 	memcpy(temp, this->buffer + start, count);
// 	temp[count] = '\0';
//
// 	glfwSetClipboardString(context->glfw_window, temp);
// }
//
//
// static void textbox_cut(Textbox* this, Context* context) {
// 	if (!textbox_hasSelection(this)) return;
//
// 	textbox_copy(this, context);
// 	textbox_deleteSelection(this);
// 	textbox_pushHistory(this);
// }
//
//
// static void textbox_paste(Textbox* this, Context* context) {
// 	const char* clip = glfwGetClipboardString(context->glfw_window);
// 	if (!clip || clip[0] == '\0') return;
//
// 	int sel_len = 0;
// 	if (textbox_hasSelection(this)) {
// 		int start, end;
// 		textbox_getSelectionRange(this, &start, &end);
// 		sel_len = end - start;
// 	}
//
// 	int available = MAX_STRING_LENGTH - (this->len - sel_len);
// 	if (available <= 0) {
// 		log_info("Buffer already max\n");
// 		return;
// 	}
//
// 	char clean[MAX_STRING_LENGTH + 1];
// 	int clean_len = 0;
// 	for (int i = 0; clip[i] != '\0' && clean_len < available; i++) {
// 		if (clip[i] == '\r') {
// 			continue;
// 		}
// 		if (clip[i] == '\n' || clip[i] == '\t' || (unsigned char)clip[i] >= 32) {
// 			clean[clean_len++] = clip[i];
// 		}
// 	}
// 	clean[clean_len] = '\0';
//
// 	if (clean_len > 0) {
// 		textbox_append(this, clean);
// 	}
// }
//
//
// static bool mouseInTextbox(const Textbox* this, double mx, double my) {
// 	return mx >= this->x && mx <= this->x + this->w &&
// 	       my >= this->y && my <= this->y + this->h;
// }
//
//
// static int posFromCoord(const Textbox* this, double mx, double my) {
// 	double char_size = 8.0 * this->font_scale;
// 	double line_height = this->line_space + char_size;
// 	if (line_height <= 0.0) return 0;
//
// 	char wrapped[TEXTBOX_MAX_STRING_LENGTH + 1];
// 	textbox_wrap(this, wrapped);
//
// 	int tar_line = (my - (this->y + padding)) / line_height;
// 	if (tar_line < 0) target_line = 0;
//
// 	int curr_line = 0, col = 0;
// 	double cx = this->x + padding;
//
// 	for (int i = 0; i < this->len; i++) {
// 		if (wrapped[i] == '\n') {
// 			if (curr_line == tar_line) {
// 				return i;
// 			}
// 			curr_line++;
// 			cx = this->x + padding;
// 			col = 0;
// 			continue;
// 		}
//
// 		int advance_col = (wrapped[i] == '\t') ? tab_advance(col) : 1;
// 		double advance_px = advance_col * char_size;
//
// 		if (curr_line == tar_line && mx < cx + advance_px * 0.5) {
// 			return i;
// 		}
//
// 		cx += advance_px;
// 		col += advance_col;
// 	}
//
// 	return this->len;
// }
//
//
// static void select_word_by_pos(Textbox* this, int pos) {
// 	if (this->len == 0) return;
// 	if (pos >= this->len) pos = this->len - 1;
// 	if (pos < 0) pos = 0;
//
// 	int start = pos;
// 	int end = pos;
//
// 	if (is_word_char(this->buffer[pos])) {
// 		while (start > 0 && is_word_char(this->buffer[start - 1])) start--;
// 		while (end < this->len && is_word_char(this->buffer[end])) end++;
// 	} else if (is_space_or_tab(this->buffer[pos])) {
// 		while (start > 0 && is_space_or_tab(this->buffer[start - 1])) start--;
// 		while (end < this->len && is_space_or_tab(this->buffer[end])) end++;
// 	} else if (this->buffer[pos] != '\n') {
// 		end = pos + 1;
// 	}
//
// 	if (start != end) {
// 		this->select_start = start;
// 		this->cursor_pos = end;
// 		sync_preferred_col(this);
// 	}
// }
//
//
// static void select_paragraph_by_pos(Textbox* this, int pos) {
// 	if (this->len == 0) return;
// 	if (pos > this->len) pos = this->len;
// 	if (pos < 0) pos = 0;
//
// 	int start = pos;
// 	if (start > 0 && start == this->len && this->buffer[start - 1] == '\n') {
// 		start--;
// 	}
// 	while (start > 0 && this->buffer[start - 1] != '\n') {
// 		start--;
// 	}
//
// 	int end = pos;
// 	while (end < this->len && this->buffer[end] != '\n') {
// 		end++;
// 	}
// 	if (end < this->len && this->buffer[end] == '\n') {
// 		end++;
// 	}
//
// 	this->select_start = start;
// 	this->cursor_pos = end;
// 	sync_preferred_col(this);
// }
//
//
// void textbox_setFontScale(Textbox* this, double font_scale) {
// 	this->font_scale = font_scale < MIN_FONT_SCALE ? MIN_FONT_SCALE : font_scale;
// 	sync_preferred_col(this);
// }
//
//
// Textbox textbox_create(int x, int y, int w, int h, RGBA fg_color, RGBA bg_color) {
// 	Textbox this = {0};
// 	this.focus = false;
// 	this.wrap_buffer[0] = '\0';
// 	this.buffer[0] = '\0'; this.len = 0;
//
// 	this.x = x; this.y = y; this.w = w; this.h = h;
// 	this.font_scale = TEXTBOX_INITIAL_FONT_SCALE;
// 	this.padding = 10.0; this.line_space = 10.0;
// 	this.fg_color = fg_color; this.bg_color = bg_color;
// 	this->sel_fg_color = 0xFFFFFFFF; this->sel_bg_color = 0x0084FFFF;
// 	this.ibeam_cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
//
// 	this.cursor_pos = 0;
// 	this.select_start = -1;
// 	this.preferred_col = 0;
//
// 	this.blink_interval_ms = TEXTBOX_BLINK_INTERVAL_MS;
// 	this.last_blink_ms = (double)get_time_ns() / 1000000.0;
// 	this.cursor_display = true;
//
// 	this.prev_mouse_down = false;
// 	this.is_dragging = false;
// 	this.last_click_time = -1.0;
// 	this.last_click_pos = -1;
// 	this.click_cnt = 0;
//
// 	this.history_cnt = 1;
// 	this->history_idx = 0;
//
// 	this->is_dirty = false;
// 	this->saved_history_idx = 0;
// 	saveSnapShot(&this->history[0], this);
//
// 	return this;
// }
//
//
// static void handle_key(Textbox* this, Context* context, int key, bool ctrl, bool shift) {
// 	posFromCoord();
// }
//
//
// void textbox_update(Textbox* this, Context* context, Event* e) {
// 	// keyboard query
// 	bool ctrl = (glfwGetKey(context->glfw_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(context->glfw_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
// 	bool shift = (glfwGetKey(context->glfw_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(context->glfw_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
//
// 	// mouse query
// 	double mx, my;
// 	glfwGetCursorPos(context->glfw_window, &mx, &my);
//
// 	bool mouse_down = (glfwGetMouseButton(context->glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
// 	bool just_pressed = mouse_down && !this->prev_mouse_down;
// 	bool just_released = !mouse_down && this->prev_mouse_down;
// 	this->prev_mouse_down = mouse_down;
//
// 	bool mouse_in_textbox = mouseInTextbox(this, mx, my);
// 	int mouse_pos = posFromCoord();
//
// 	// time query
// 	double now_ms = (double)getTimeNs() / 1000000.0;
//
// 	// set cursor
// 	if (mouseInTextbox || this->is_dragging) glfwSetCursor(context->glfw_window, this->ibeam_cursor);
// 	else glfwSetCursor(context->glfw_window, NULL);
//
// 	// textbox focus
// 	if (just_pressed && mouseInTextbox) { this->focus = true; this->cursor_display = true;}
// 	if (glfwGetKey(context->glfw_window, GLFW_KEY_SLASH) == GLFW_PRESS) {this->focus = true; this->cursor_display = true;}
// 	if (glfwGetKey(context->glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
// 		this->focus = false;
// 		this->cursor_display = false;
// 		this->select_start = -1;
// 	}
// 	if (this->focus == false) return;
//
//
// 	// blinking cursor
// 	if (now_ms - this->last_blink_ms >= this->blink_interval_ms) {
// 		this->cursor_display = !this->cursor_display;
// 		this->last_blink_ms = now_ms;
// 	}
//
// 	// typed characters
// 	if (e->char_cnt > 0) {
// 		char text[TEXTBOX_MAX_FRAME_EVENT + 1];
// 		int cnt = 0;
// 		for (int i = 0; i < e->char_cnt; i++) {
// 			if (e->chars[i] >= 32 && e->chars[i] < 127) {
// 				text[cnt++] = (char)e->chars[i];
// 			}
// 		}
// 		text[cnt] = '\0';
// 		if (cnt > 0) {
// 			append(this, text);
// 			this->last_blink_ms = now_ms;
// 			this->cursor_display = true;
// 		}
// 	}
//
// 	// mouse operation
// 	if (just_pressed && mouseInTextbox) {
// 		if (!shift && (now_ms - this->last_click_time_ms <= TEXTBOX_CLICK_THRESHOLD_MS) && mouse_pos == this->last_click_pos) {
// 			this->click_count = (this->click_count % 3) + 1;
// 		} else {
// 			this->click_count = 1;
// 		}
//
// 		this->last_click_time_ms = now_ms;
// 		this->last_click_pos = mouse_pos;
//
// 		if (this->click_count == 3) {
// 			select_paragraph_by_pos(this, mouse_pos);
// 			this->is_dragging = false;
// 		} else if (this->click_count == 2) {
// 			select_word_by_pos(this, mouse_pos);
// 			this->is_dragging = false;
// 		} else if (shift) {
// 			if (this->select_start == -1) {
// 				this->select_start = this->cursor_pos;
// 			}
// 			this->cursor_pos = mouse_pos;
// 			this->is_dragging = true;
// 			sync_preferred_col(this);
// 		} else {
// 			this->cursor_pos = mouse_pos;
// 			this->select_start = mouse_pos;
// 			this->is_dragging = true;
// 			sync_preferred_col(this);
// 		}
// 	}
//
// 	if (this->is_dragging && mouse_down) {
// 		if (mouse_pos != this->cursor_pos) {
// 			this->cursor_pos = mouse_pos;
// 			sync_preferred_col(this);
// 		}
// 	}
//
// 	if (just_released && this->is_dragging) {
// 		this->is_dragging = false;
// 		if (this->select_start == this->cursor_pos) {
// 			this->select_start = -1;
// 		}
// 	}
//
// 	for (int i = 0; i < e->key_cnt; i++) {
// 		if (e->keys[i].action == GLFW_PRESS || e->keys[i].action == GLFW_REPEAT) {
// 			if (key == GLFW_KEY_TAB) {
// 				textbox_tab(this, shift);
// 			}
// 			if (key == GLFW_KEY_A && ctrl) {
// 				textbox_selectAll(this);
// 			}
// 			if (key == GLFW_KEY_ENTER) {
// 				textbox_append(this, "\n");
// 			}
// 			if (key == GLFW_KEY_BACKSPACE) {
// 				if (ctrl) textbox_cbackspace(this);
// 				else textbox_backspace(this);
// 			}
// 			if (key == GLFW_KEY_DELETE) {
// 				textbox_delete(this);
// 			}
// 			if (key == GLFW_KEY_LEFT) {
// 				if (!shift && !ctrl && textbox_hasSelection(this)) {
// 					int start, end;
// 					textbox_getSelectionRange(this, &start, &end);
// 					this->cursor_pos = start;
// 					this->select_start = -1;
// 					sync_preferred_col(this);
// 				} else {
// 					textbox_beginNav(this, shift);
// 					if (ctrl) textbox_cleft(this);
// 					else textbox_left(this);
// 				}
// 			}
// 			if (key == GLFW_KEY_RIGHT) {
// 				if (!shift && !ctrl && textbox_hasSelection(this)) {
// 					int start, end;
// 					textbox_getSelectionRange(this, &start, &end);
// 					this->cursor_pos = end;
// 					this->select_start = -1;
// 					sync_preferred_col(this);
// 				} else {
// 					textbox_beginNav(this, shift);
// 					if (ctrl) textbox_cright(this);
// 					else textbox_right(this);
// 				}
// 			}
// 			if (key == GLFW_KEY_UP) {
// 				textbox_beginNav(this, shift);
// 				textbox_moveVertical(this, -1);
// 			}
// 			if (key == GLFW_KEY_DOWN) {
// 				textbox_beginNav(this, shift);
// 				textbox_moveVertical(this, +1);
// 			}
// 			if (key == GLFW_KEY_EQUAL) {
// 				if (ctrl) textbox_setFontScale(this, this->font_scale + 1.0);
// 			}
// 			if (key == GLFW_KEY_MINUS) {
// 				if (ctrl) textbox_setFontScale(this, this->font_scale - 1.0);
// 			}
// 			if (key == GLFW_KEY_V && ctrl) {
// 				textbox_paste(this, context);
// 			}
// 			if (key == GLFW_KEY_C && ctrl) {
// 				textbox_copy(this, context);
// 			}
// 			if (key == GLFW_KEY_X && ctrl) {
// 				textbox_cut(this, context);
// 			}
// 			if (key == GLFW_KEY_Z && ctrl) {
// 				if (shift) textbox_redo(this);
// 				else textbox_undo(this);
// 			}
// 			this->last_blink_ms = now_ms;
// 			this->cursor_display = true;
// 		}
// 	}
// }
//
//
// void textbox_render(Textbox* this, Renderer* renderer) {
// 	// background
// 	renderer_fillRect(renderer, this->x, this->y, this->w, this->h, this->bg_color);
//
// 	// text
// 	double padding = 10.0;
// 	double char_size = 8.0 * this->font_scale;
// 	double line_space = 10.0;
//
// 	char buffer[MAX_STRING_LENGTH + 1];
// 	textbox_wrap(this, buffer);
//
// 	int sel_start = -1, sel_end = -1;
// 	bool has_sel = textbox_hasSelection(this);
// 	if (has_sel) {
// 		textbox_getSelectionRange(this, &sel_start, &sel_end);
// 	}
//
// 	double cx = this->x + padding, cy = this->y + padding;
// 	double cursor_x = cx, cursor_y = cy;
// 	int col = 0;
//
// 	for (int i = 0; i < this->len; i++) {
// 		if (i == this->cursor_pos) {
// 			cursor_x = cx;
// 			cursor_y = cy;
// 		}
//
// 		int advance_cols = (buffer[i] == '\t') ? textbox_tabAdvance(col) : 1;
// 		double advance_px = advance_cols * char_size;
//
// 		// selection highlight
// 		bool in_sel_range = has_sel && i >= sel_start && i < sel_end;
// 		if (in_sel_range) {
// 			double hl_w = (buffer[i] == '\n') ? (char_size * 0.5) : advance_px;
// 			renderer_fillRect(renderer, cx, cy, hl_w, char_size + line_space * 0.5, this->sel_bg_color);
// 		}
//
// 		if (buffer[i] == '\n') {
// 			cx = this->x + padding;
// 			cy += char_size + line_space;
// 			col = 0;
// 		} else if (buffer[i] == '\t') {
// 			cx += advance_px;
// 			col += advance_cols;
// 		} else {
// 			renderer_drawChar(renderer, buffer[i], cx, cy, this->font_scale, in_sel_range ? this->sel_fg_color : this->fg_color);
// 			cx += char_size;
// 			col += 1;
// 		}
// 	}
// 	if (this->cursor_pos >= this->len) {
// 		cursor_x = cx;
// 		cursor_y = cy;
// 	}
//
// 	// cursor
// 	if (this->cursor_display && this->focus) renderer_fillRect(renderer, cursor_x, cursor_y, 4.0, char_size, this->fg_color);
//
// 	// text length
// 	int lx = 1180, ly = 875;
// 	static char lbuffer[64] = {0};
//
// 	int lsel_len = textbox_getSelectionLength(this);
// 	const char* save_state = !this->is_dirty ? "Saved!" : "(Unsave)";
// 	snprintf(lbuffer, sizeof(lbuffer), "%s Char:Cursor:Sel:Max %d:%d:%d:%d", save_state, this->len, this->cursor_pos, lsel_len, MAX_STRING_LENGTH);
//
// 	for (int i = strlen(lbuffer)-1; i >= 0; i--) {
// 		lx -= 8.0 * 2.0;
// 		renderer_drawChar(renderer, lbuffer[i], lx, ly, 2, RGBA_BLACK);
// 	}
// }
//
//
