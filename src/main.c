#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_keysyms.h>

#include <X11/keysym.h>

#include "global.h"
#include "util.h"
#include "client.h"
#include "list.h"
#include "keyboard.h"
#include "config.h"
#include "luabinding.h"

/* func declarations */
void events_loop(void);
void setup_callbacks();

/* Helper functions */
client_t* find_window(xcb_window_t window);
void remove_window(xcb_window_t window);

/* globals */
xcb_connection_t *c = NULL;
xcb_screen_t *screen = NULL;
xcb_window_t root = 0;
list_head_t *clients = NULL;
client_t *focused = NULL;
CONFIG cfg;

/* Callbacks */
lb_func on_setup;
lb_func on_new_window;
lb_func on_destroy_window;
lb_func on_key_press;

void events_loop(void)
{
	xcb_generic_event_t *event;
	while((event = xcb_wait_for_event(c)))
	{
		switch(event->response_type & ~0x80)
		{
			// need to figure out what's up with event 0
			case 0:
			{
				break;
			}
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
					lb_push_func(on_new_window);
					lb_new_window(&client, e->window);
					list_item_t *item = list_new_item(clients);
					item->data = (void*)client;
					lb_call(1);
				}

				set_window_border(c, client, cfg.border_width, cfg.border_normal_color);

				xcb_flush(c);

				break;
			}
			case XCB_DESTROY_NOTIFY:
			{
				pdebug("Window destroyed");
				xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t*) event; 

				lb_remove_window(find_window(e->window));
				remove_window(e->window);

				if (clients->start) {
					focus_window(c, clients->start->data, &focused);
					xcb_flush(c);
				} else {
					focused = NULL;
				}

				break;
			}
			case XCB_KEY_PRESS:
			{
				xcb_key_press_event_t *e = (xcb_key_press_event_t*) event;
				// temp to reload config
				if (e->detail == 27) {
					lb_load_config("src/lua/config.lua", &cfg);
				}

				break;
			}
			default:
				printf("I got an event of type: %d\n", event->response_type);
				break;
		}
		free(event);
	}
}

void setup_callbacks()
{
	on_setup = lb_is_callback("setup");;
	on_new_window = lb_is_callback("new_window");
	on_key_press = lb_is_callback("key_press");
	on_destroy_window = lb_is_callback("destroy_window");
	pdebug("on_setup: %p", on_setup);
	pdebug("on_new_window: %p", on_new_window);
}


/* Helper functions */
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
/* End helper functions */

int main(int argc, char** argv)
{
	int screen_nbr;
	xcb_screen_iterator_t iter;

	cfg = DEFAULT_CONFIG;

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
	};

	xcb_change_window_attributes_checked(c, root, mask, values);

	xcb_flush(c);

	log_info("Information of screen: %d", screen->root);
	log_info("Width: %d", screen->width_in_pixels);
	log_info("Height: %d", screen->height_in_pixels);
	log_info("White pixel: %d", screen->white_pixel);
	log_info("Black pixel: %d", screen->black_pixel);

	clients = list_new();

	keypress_t keys[] = {
		{ "r", 0, XCB_MOD_MASK_CONTROL },
	};

	grab_keys(c, keys, sizeof(keys) / sizeof(keypress_t), root);

	xcb_flush(c);

	lb_init(c, root, &focused);
	/* TODO: find a better way to test config files */
	lb_load_config("src/lua/config.lua", &cfg);

	setup_callbacks();

	lb_push_func(on_setup);
	lb_call(0);

	events_loop();


error:

	return 0;
}
