#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>

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
void setup(const char* cfg_file);
void cleanup();
void on_maprequest(xcb_window_t window);

/* Helper functions */
client_t* find_window(xcb_window_t window);
void remove_window(xcb_window_t window);
void get_atoms();

/* globals */
xcb_connection_t *c = NULL;
xcb_screen_t *screen = NULL;
xcb_window_t root = 0;
list_head_t *clients = NULL;
client_t *focused = NULL;
CONFIG cfg;
keypress_t *keys = NULL;
const char* cfg_f;

static char *WM_ATOM_NAMES[] = { "WM_PROTOCOLS", "WM_DELETE_WINDOW" };
xcb_atom_t wm_atoms[WM_COUNT];

/* Callbacks */
lb_func on_setup;
lb_func on_new_window;
lb_func on_destroy_window;

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
			case XCB_BUTTON_PRESS:
			{
				xcb_button_press_event_t *e = (xcb_button_press_event_t*) event;
				pdebug("Button pressed: %d, %d", e->detail,
						e->event);

				client_t *client = find_window(e->event);
				if (client) {
					focus_window(c, client, &focused);
				}

				xcb_allow_events(c, XCB_ALLOW_REPLAY_POINTER, e->time);
				xcb_flush(c);

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
				on_maprequest(e->window);
				break;
			}
			case XCB_DESTROY_NOTIFY:
			{
				pdebug("Window destroyed");
				xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t*) event;

				if (clients->start) {
					focus_window(c, clients->start->data, &focused);
					xcb_flush(c);
				} else {
					focused = NULL;
				}

				lb_push_func(on_destroy_window);
				lb_push_number(e->window);
				lb_call(1);

				lb_remove_window(find_window(e->window));
				remove_window(e->window);
				break;
			}
			case XCB_KEY_PRESS:
			{
				xcb_key_press_event_t *e = (xcb_key_press_event_t*) event;
				// temp to reload config
				if (e->detail == 27) {
					cleanup();
					setup(cfg_f);
				}

				for (int i = 0; i < lb_get_keycount(); i++) {
					if (keys[i].kc == e->detail && keys[i].mask == e->state) {
						lb_on_keypress(keys[i].callback_index);
						break;
					}
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
	on_destroy_window = lb_is_callback("destroy_window");
}


void setup(const char* cfg_file)
{
	xcb_query_tree_cookie_t cookie;
	xcb_query_tree_reply_t *reply;
	const char *dir;
	char path_copy[300];
	xcb_window_t *windows;
	int i;

	cfg = DEFAULT_CONFIG;

	keys = malloc(sizeof(keypress_t) * 256); // COMPLETELY ARBITRARY AND VERY STOOPID
	check(keys, "failed to malloc keys");

	clients = list_new();

	get_atoms();

	memset(path_copy, 0, sizeof(path_copy));
	strncpy(path_copy, cfg_file, sizeof(path_copy));
	dir = dirname(path_copy);
	chdir(dir);

	lb_init(screen->width_in_pixels, screen->height_in_pixels);
	lb_load_config(cfg_file, &cfg);

	setup_callbacks();
	lb_push_func(on_setup);
	lb_call(0);

	grab_keys(c, keys, lb_get_keycount(), root);

	/* Find all windows under the root window and show them to lua */
	cookie = xcb_query_tree(c, root);
	reply = xcb_query_tree_reply(c, cookie, NULL);
	check(reply, "Failed to get windows");

	windows = xcb_query_tree_children(reply);
	for (i = 0; i < reply->children_len; i++)
		on_maprequest(windows[i]);

	xcb_flush(c);

	if (reply) free(reply);
error:
	return;
}

void cleanup()
{
	if (keys) free(keys);
	lb_cleanup();
	list_clear(clients);
	on_setup = NULL;
	on_new_window = NULL;
	on_destroy_window = NULL;
	xcb_ungrab_key(c, XCB_GRAB_ANY, root, XCB_MOD_MASK_ANY);
	xcb_flush(c);
}

void on_maprequest(xcb_window_t window)
{
	xcb_get_window_attributes_cookie_t attr_cookie;
	xcb_get_window_attributes_reply_t *attr_reply;
	client_t *client;

	attr_cookie = xcb_get_window_attributes(c, window);
	attr_reply = xcb_get_window_attributes_reply(c, attr_cookie, NULL);
	check(attr_reply, "Failed to get window attributes");
	if (attr_reply->override_redirect) {
		free(attr_reply);
		return;
	}

	client = find_window(window);

	if (!client) {
		lb_push_func(on_new_window);
			lb_new_window(&client, window);
			list_item_t *item = list_new_item(clients);
			item->data = (void*)client;
			client->border_width = cfg.border_width;
		lb_call(1);

		xcb_grab_button(c, 1, client->window,
				XCB_EVENT_MASK_BUTTON_PRESS,
				XCB_GRAB_MODE_SYNC,
				XCB_GRAB_MODE_ASYNC,
				XCB_WINDOW_NONE,
				XCB_CURSOR_NONE,
				XCB_BUTTON_INDEX_ANY,
				XCB_MOD_MASK_ANY);
	}

	set_window_border(c, client, cfg.border_width, cfg.border_normal_color);

	xcb_flush(c);
error:
	if (attr_reply) free(attr_reply);
	return;
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

void get_atoms()
{
	xcb_intern_atom_cookie_t cookies[WM_COUNT];
	xcb_intern_atom_reply_t *reply;

	for (int i = 0; i < WM_COUNT; i++)
		cookies[i] = xcb_intern_atom(c, 0, strlen(WM_ATOM_NAMES[i]),
			WM_ATOM_NAMES[i]);

	for (int i = 0; i < WM_COUNT; i++) {
		reply = xcb_intern_atom_reply(c, cookies[i], NULL);
		if (reply) {
			wm_atoms[i] = reply->atom;
			free(reply);
		} else {
			log_warn("Failed to get atom %s", WM_ATOM_NAMES[i]);
		}
	}
}

/* End helper functions */

int main(int argc, char** argv)
{
	int screen_nbr;
	xcb_screen_iterator_t iter;

	if (argc < 2) {
		printf("No config file specified!");
		exit(1);
	}

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

	uint32_t mask = XCB_CW_EVENT_MASK;
	uint32_t values[] = {
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
		| XCB_EVENT_MASK_STRUCTURE_NOTIFY
		| XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
	};

	xcb_change_window_attributes_checked(c, root, mask, values);

	log_info("Information of screen: %d", screen->root);
	log_info("Width: %d", screen->width_in_pixels);
	log_info("Height: %d", screen->height_in_pixels);
	log_info("White pixel: %d", screen->white_pixel);
	log_info("Black pixel: %d", screen->black_pixel);

	cfg_f = argv[1];
	setup(cfg_f);
	events_loop();
	cleanup();

error:

	return 0;
}
