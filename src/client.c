#include "client.h"

#include "util.h"

void move_window(xcb_connection_t *c, client_t client, uint16_t x, uint16_t y)
{
	check(c, "connection is null");

	client.x = x;
	client.y = y;
	uint32_t values[] = {x, y};

	 xcb_configure_window(c, client.window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

error:
	return;
}

void move_window_relative(xcb_connection_t *c, client_t client, uint16_t x, uint16_t y)
{
	move_window(c, client, client.x + x, client.y + y);
}

void resize_window(xcb_connection_t *c, client_t client, uint16_t width, uint16_t height)
{
	check(c, "connection is null");

	client.width = width;
	client.height = height;
	uint32_t values[] = { width, height};

	xcb_configure_window(c, client.window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);

error:
	return;
}

void map_window(xcb_connection_t *c, client_t client)
{
	check(c, "connection is null");

	xcb_map_window(c, client.window);
	client.mapped = TRUE;
error:
	return;
}

void unmap_window(xcb_connection_t *c, client_t client)
{
	check(c, "connection is null");

	xcb_unmap_window(c, client.window);

	client.mapped = FALSE;
error:
	return;
}

void toggle_map_window(xcb_connection_t *c, client_t client)
{
	if (client.mapped)
		unmap_window(c, client);
	else 
		map_window(c, client);
}

