#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "util.h"
#include "client.h"
#include "list.h"

#define BORDER_WIDTH 3
#define BORDER_COLOR 0x333333

/* func declarations */
void events_loop(void);

/* Helper functions */
client_t* find_window(xcb_window_t window);
void remove_window(xcb_window_t window);

/* globals */
xcb_connection_t *c;
xcb_screen_t *screen;
xcb_window_t root;
list_head_t* clients;

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

				client_t *client;
				client = find_window(e->window);
				if (!client) {
					client = new_window(c, e->window);
					list_item_t *item = list_new_item(clients);
					item->data = (void*)client;
				}

				move_window(c, *client, 30, 30);
				resize_window(c, *client, 400, 400);
				set_window_border(c, *client, BORDER_WIDTH, BORDER_COLOR);
				map_window(c, *client);

				xcb_flush(c);

				break;
			}
			case XCB_DESTROY_NOTIFY:
			{
				pdebug("Window destroyed");
				xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t*) event; 
				remove_window(e->window);
				break;
			}
			default:
				printf("I got an event of type: %d\n", event->response_type);
				break;
		}
		free(event);
	}
}

client_t* find_window(xcb_window_t window)
{
	list_item_t *i;

	for (i = clients->start; i != NULL; i = i->next) {
		if (((client_t*)i->data)->window == window)
			return (client_t*)i->data;
	}

	return NULL;
}

void remove_window(xcb_window_t window)
{
	list_item_t *i;
	for (i = clients->start; i != NULL; i = i->next) {
		if (((client_t*)i->data)->window == window) {
			list_del_item(clients, i);
			break;
		}
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

	clients = list_new();

	events_loop();


error:

	return 0;
}