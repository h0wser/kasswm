#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "util.h"
#include "client.h"

/* func declarations */
void events_loop(void);

/* globals */
xcb_connection_t *c;
xcb_screen_t *screen;
xcb_window_t root;

void events_loop(void)
{
	xcb_generic_event_t *event;
	while((event = xcb_wait_for_event(c)))
	{
		switch(event->response_type & ~0x80)
		{
			case XCB_CREATE_NOTIFY:
			{
				pdebug("Window created");

				break;
			}
			case XCB_MAP_REQUEST:
			{
				pdebug("Window requested map");

				xcb_map_request_event_t *e = (xcb_map_request_event_t*) event;
				xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(c, xcb_get_geometry(c, e->window), 0);

				client_t client;
				client.window = e->window;

				move_window(c, client, 30, 30);
				resize_window(c, client, 400, 400);
				map_window(c, client);

				xcb_flush(c);

				free(geom);
				break;
			}
			default:
				printf("I got an event of type: %d\n", event->response_type);
				break;
		}
		free(event);
	}
}


int main(int argc, char** argv)
{
	int screen_nbr;
	xcb_screen_iterator_t iter;

	// open connection
	c = xcb_connect(NULL, &screen_nbr);
	check(c, "connection is null");

	iter = xcb_setup_roots_iterator(xcb_get_setup(c));
	for (; iter.rem; --screen_nbr, xcb_screen_next(&iter))
		if (screen_nbr == 0)
		{
			screen = iter.data;
			break;
		}

	root = screen->root;

	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[] = {
		screen->white_pixel,
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY 
		| XCB_EVENT_MASK_STRUCTURE_NOTIFY 
		| XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT 
		| XCB_EVENT_MASK_BUTTON_PRESS 
	};

	xcb_change_window_attributes_checked(c, root, mask, values);

	xcb_flush(c);

	log_info("Information of screen: %d", screen->root);
	log_info("Width: %d", screen->width_in_pixels);
	log_info("Height: %d", screen->height_in_pixels);
	log_info("White pixel: %d", screen->white_pixel);
	log_info("Black pixel: %d", screen->black_pixel);

	events_loop();

error:


	return 0;
}
