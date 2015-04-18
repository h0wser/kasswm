#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <xcb/xcb_keysyms.h>

#include <X11/keysym.h>

typedef struct key_t
{
	char* name; /* string rep of key */
	xcb_keycode_t keycode;
} key_t;

int grab_keys(xcb_connection_t *c, key_t* keys, int keyc, xcb_window_t root);

#endif /* KEYBOARD_H */
