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

void lb_load_config(const char *filename, CONFIG *cfg)
{
	if (luaL_dofile(L, filename)) {
		log_warn("error loading config file %s", filename);
	}

	lb_get_table("config");

	cfg->border_width = lb_get_number("border_width");
	cfg->border_normal_color = lb_get_number("border_normal_color");
}


void lb_get_table(const char *name)
{
	lua_getglobal(L, name);
}

int lb_get_number(const char *name)
{
	int result = 0;
	lua_pushstring(L, name);
	lua_gettable(L, -2);
	check(lua_isnumber(L, -1), "Lua error, value %s is not a number", name);

	result = lua_tonumber(L, -1);

	lua_pop(L, 1);

error:
	return result;
}

const char* lb_get_string(const char *name)
{
	const char* result = 0;
	lua_pushstring(L, name);
	lua_gettable(L, -2);
	check(lua_isstring(L, -1), "Lua error, value %s is not a string", name);

	result = lua_tostring(L, -1);

	lua_pop(L, 1);

error:
	return result;
}

int lb_call(const char *name, ...)
{
	return 1;
}

void lb_set_context(xcb_connection_t *con, xcb_window_t p_root)
{

}
