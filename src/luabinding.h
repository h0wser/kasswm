#ifndef LUABINDING_H
#define LUABINDING_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "client.h"
#include "keyboard.h"
#include "list.h"
#include "config.h"

/* All lua functions are prefixed "lb_" (lua binding) */

/* Globals */
lua_State *L;
xcb_connection_t *c;
xcb_window_t root;

void lb_init();
void lb_load_config(const char *filename, CONFIG *cfg);

/* Gets a table and puts it on the stack */
void lb_get_table(const char *name);

/* Returns values from a table */
int lb_get_number(const char *name);
const char* lb_get_string(const char *name);

/* Call a function defined in lua */
int lb_call(const char *name, ...);

/* Expose functions to lua */
void lb_set_context(xcb_connection_t *con, xcb_window_t p_root);


#endif /* LUABINDING_H */
