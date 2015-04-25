#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <xcb/xcb_keysyms.h>

#include <X11/keysym.h>

typedef const char* lb_func;

/* 
 * name can be a alphanumeric key, function key, space, tab, ,(comma), 
 * .(period), arrow keys, home, pg-up, pg-down
 * 
 */
typedef struct keypress_t
{
	const char* name; /* string rep of key */
	xcb_keysym_t keysym;
	uint16_t mask;
	lb_func callback;
} keypress_t;

int grab_keys(xcb_connection_t *c, keypress_t* keys, int keyc, xcb_window_t root);

#endif /* KEYBOARD_H */
