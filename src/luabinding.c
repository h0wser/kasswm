#include "luabinding.h"

#include "util.h"

void lb_init()
{
	L = luaL_newstate();
	check(L, "Failed to get new lua state");
	luaL_openlibs(L);

error:
	/* die here? */
	return;
}

void lb_load_config(char *filename, CONFIG *cfg)
{
	if (luaL_dofile(L, filename)) {
		log_warn("error loading config file %s", filename);
	}

	lua_getglobal(L, "border_normal_color");
	lua_getglobal(L, "border_width");

	if (!lua_isnumber(L, -2))
		log_error("border_normal_color needs to be a number");
	else
		cfg->border_normal_color = (uint32_t)lua_tonumber(L, -2);

	if (!lua_isnumber(L, -1))
		log_error("border_width needs to be a number");
	else
		cfg->border_width = (uint32_t)lua_tonumber(L, -1);
}
