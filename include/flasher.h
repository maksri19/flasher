#ifndef FLASHER_H
#define FLASHER_H

#include "common.h"
#include "textbox.h"

#define FLASHER_MAX_FILE_COUNT	(1024)
#define FLASHER_FILE_NAME_LENGTH (10)
#define FLASHER_MAX_FILE_PATH_LENGTH (255)
#define FLASHER_FILE_EXTENSION	".flasher"


typedef struct Flasher Flasher;

struct Flasher {
	// file managerment
	int		flasher_cnt;
	int		flasher_idx;
	char	flashers[FLASHER_MAX_FILE_COUNT * (FLASHER_FILE_NAME_LENGTH+1)];
	int		stride;

	// textbox
	Textbox	textbox;
	bool	textbox_focus;
	bool	textbox_just_focus;

	// mouse
	bool	mouse_down_prev;
};


Flasher	flasher_create(void);
void	flasher_update(Flasher* this, Context* ctx, Event* e);
void 	flasher_render(Flasher* this, Renderer* renderer);


#endif
