#ifndef CLIENT_H
#define CLIENT_H

#include <xcb/xproto.h>

typedef struct client_t
{
	xcb_window_t window;
	uint16_t x, y;
	uint16_t width, height;
	uint8_t depth;
	int mapped;
	int dankness;
	uint16_t border_width;
} client_t;

/* None of these functions use xcb_flush(c).
 * You must call xcb_flush yourself after calling client functions
 * This is in order to be able execute multiple functions before flushing
 */

void move_window(xcb_connection_t *c, client_t *client, uint16_t x, uint16_t y);
void move_window_relative(xcb_connection_t *c, client_t *client, uint16_t x, uint16_t y);

void resize_window(xcb_connection_t *c, client_t *client, uint16_t width, uint16_t height);

void map_window(xcb_connection_t *c, client_t *client);
void unmap_window(xcb_connection_t *c, client_t *client);
void toggle_map_window(xcb_connection_t *c, client_t *client);

void set_window_border(xcb_connection_t *c, client_t *client, uint16_t border_width, int color);

void focus_window(xcb_connection_t *c, client_t *client, client_t **focused);

void new_window(xcb_connection_t *c, client_t *client, xcb_window_t window);
void destroy_window(xcb_connection_t *c, client_t *client);

#endif /* CLIENT_H */
