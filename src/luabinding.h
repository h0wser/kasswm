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

void lb_init();
void lb_load_config(char *filename, CONFIG *cfg);


#endif /* LUABINDING_H */
