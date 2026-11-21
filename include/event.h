#ifndef EVENT_H
#define EVENT_H

#include "common.h"
#include "context.h"

#define EVENT_MAX_FRAME_EVENT 16


typedef struct Event Event;
typedef struct EventKey EventKey;

struct EventKey {
	int	key;
	int	scancode;
	int action;
	int mods;
};

struct Event {
	unsigned int	chars[EVENT_MAX_FRAME_EVENT];
	EventKey		keys[EVENT_MAX_FRAME_EVENT];
	int				char_cnt;
	int				key_cnt;
	bool			mouse_pressl;
	bool			mouse_pressr;
};


void	event_setCallback(Event* e, Context* ctx);
void 	event_clear(Event* this);



#endif
