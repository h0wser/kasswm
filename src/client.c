#include "client.h"

#include "util.h"

void move_window(xcb_connection_t *c, client_t *client, uint16_t x, uint16_t y)
{
	check(c, "connection is null");

	client->x = x;
	client->y = y;
	uint32_t values[] = {x, y};

	 xcb_configure_window(c, client->window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

error:
	return;
}

void move_window_relative(xcb_connection_t *c, client_t *client, uint16_t x, uint16_t y)
{
	move_window(c, client, client->x + x, client->y + y);
}

void resize_window(xcb_connection_t *c, client_t *client, uint16_t width, uint16_t height)
{
	check(c, "connection is null");

	client->width = width;
	client->height = height;
	uint32_t values[] = { width, height};

	xcb_configure_window(c, client->window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);

error:
	return;
}

void map_window(xcb_connection_t *c, client_t *client)
{
	check(c, "connection is null");

	xcb_map_window(c, client->window);
	client->mapped = TRUE;
error:
	return;
}

void unmap_window(xcb_connection_t *c, client_t *client)
{
	check(c, "connection is null");

	xcb_unmap_window(c, client->window);

	client->mapped = FALSE;
error:
	return;
}

void toggle_map_window(xcb_connection_t *c, client_t *client)
{
	if (client->mapped)
		unmap_window(c, client);
	else 
		map_window(c, client);
}

void set_window_border(xcb_connection_t *c, client_t *client, uint16_t border_width, int color)
{
	check(c, "connection can't be null");	
	uint32_t values[] = { border_width };
	xcb_configure_window(c, client->window, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);

	values[0] = color;
	xcb_change_window_attributes(c, client->window, XCB_CW_BORDER_PIXEL, values);


error:
	return;
}

void focus_window(xcb_connection_t *c, client_t *client, client_t **focused)
{
	check(c, "connection can't be null");	
	check(client, "client can't be null");

	xcb_set_input_focus(c, XCB_INPUT_FOCUS_POINTER_ROOT, client->window, XCB_CURRENT_TIME);
	*focused = client;

error:
	return;
}

client_t* new_window(xcb_connection_t *c, xcb_window_t window)
{
	xcb_get_geometry_reply_t *geom = NULL;
	xcb_get_geometry_cookie_t cookie;
	client_t* client = NULL;

	client = malloc(sizeof(client_t));
	check(client, "Failed to malloc client");

	client->window = window;
	cookie = xcb_get_geometry_unchecked(c, window);
	geom = xcb_get_geometry_reply(c, cookie, 0);
	check(geom, "Failed to get window geometry");

	client->depth = geom->depth;
	client->x = geom->x;
	client->y = geom->y;
	client->width = geom->height;
	client->height = geom->height;
	client->mapped = FALSE;

error: 
	if (geom) free(geom);
	return client;
}

void destroy_window(xcb_connection_t *c, client_t *client)
{

}

