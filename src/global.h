#ifndef GLOBAL_H
#define GLOBAL_H

#include <xcb/xcb.h>

#include "client.h"
#include "list.h"
#include "config.h"
#include "keyboard.h"

extern xcb_connection_t *c;
extern xcb_screen_t *screen;
extern xcb_window_t root;
extern list_head_t *clients;
extern client_t *focused;
extern CONFIG cfg;
extern keypress_t *keys;

#endif /* GLOBAL_H */
