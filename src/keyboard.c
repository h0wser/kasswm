#include "keyboard.h"

#include "util.h"

#include <string.h>

static xcb_keysym_t string_to_keysym(char* name);

int grab_keys(xcb_connection_t *c, key_t* keys, int keyc, xcb_window_t root)
{
	xcb_key_symbols_t *keysyms = NULL;
	xcb_keycode_t *kc = NULL;
	xcb_keysym_t sym;

	int i;

	check(c, "c can't be null");

	keysyms = xcb_key_symbols_alloc(c);
	check(keysyms, "Failed to alloc keysyms");
	for (i = 0; i < keyc; ++i) {
		sym = string_to_keysym(keys[i].name);
		if (sym == XK_VoidSymbol) {
			log_error("Invalid keyname: %s", keys[i].name);
			continue;
		}
		
		kc = xcb_key_symbols_get_keycode(keysyms, sym);
		xcb_grab_key(c, 1, root, XCB_MOD_MASK_ANY, *kc, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
		keys[i].keycode = *kc;

		free(kc);
	}

	return 0;

error:
	if (kc) free(kc);
	if (keysyms) free(keysyms);
	return 1;
}

/* I feel that there should be a better way of doing this */
/* BEGIN DUMB */

typedef struct { char* name; xcb_keysym_t keysym; } sym;

static sym lookuptable[] = {
	{ "a", XK_a },
	{ "b", XK_b },
	{ "c", XK_c },
	{ "d", XK_d },
	{ "e", XK_e },
	{ "f", XK_f },
	{ "g", XK_g },
	{ "h", XK_h },
	{ "i", XK_i }
};

#define N_ELEM sizeof(lookuptable) / sizeof(sym)

static xcb_keysym_t string_to_keysym(char* name)
{
	int i;
	
	for (i = 0; i < N_ELEM; ++i) {
		if (!strcmp(lookuptable[i].name, name)) {
			return lookuptable[i].keysym;
		}
	}
	return XK_VoidSymbol;
}

/* END DUMB */
