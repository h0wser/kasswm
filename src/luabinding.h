#ifndef LUABINDING_H
#define LUABINDING_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "client.h"
#include "keyboard.h"
#include "list.h"
#include "config.h"

#define TABLENAME "kass"

/* All lua functions are prefixed "lb_" (lua binding) */

typedef const char* lb_func;

/* Globals */
lua_State *L;
xcb_connection_t *c;
xcb_window_t root;

void lb_init(xcb_connection_t *con, xcb_window_t p_root);
void lb_load_config(const char *filename, CONFIG *cfg);

/* Gets a table and puts it on the stack */
/* returns 0 on success and 1 on failure */
int lb_get_table(const char *name);

/* Returns values from a table */
int lb_get_number(const char *name);
const char* lb_get_string(const char *name);

/* Call a function defined in lua */
/* returns 0 on success and 1 on failure */
int lb_call(lb_func name);

/* Expose functions to lua */
lb_func lb_is_callback(const char *name);


#endif /* LUABINDING_H */
