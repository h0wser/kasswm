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

/* Window functions */
/* Puts a new window table on the stack with data from client */
void lb_create_window(client_t *client);
void lb_add_window(client_t *client);
void lb_remove_window(client_t *client);

/* Call a function defined in lua
* First use lb_push_func to push the function name to the stack
* Then push all argumnets
* Call lb_call with the number of args to execute the function
* returns 0 on success and 1 on failure */
int lb_push_func(lb_func name);
int lb_call(int nargs);

/* Expose functions to lua */
lb_func lb_is_callback(const char *name);


#endif /* LUABINDING_H */
