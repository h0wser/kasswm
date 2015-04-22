#include "luabinding.h"

#include "util.h"

void lb_init(xcb_connection_t *con, xcb_window_t p_root)
{
	L = luaL_newstate();
	check(L, "Failed to get new lua state");
	luaL_openlibs(L);

	c = con;
	root = p_root;

	/* Create the kass table in lua */
	lua_newtable(L);
	lua_setglobal(L, TABLENAME);

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


int lb_get_table(const char *name)
{
	lua_getglobal(L, name);
	check(lua_istable(L, -1), "%s is not a table", name);

	return 0;
error:
	return 1;
}

int lb_get_number(const char *name)
{
	int result = 0;
	lua_getfield(L, -1, name);
	check(lua_isnumber(L, -1), "Lua error, value %s is not a number", name);

	result = lua_tonumber(L, -1);

	lua_pop(L, 1);

error:
	return result;
}

const char* lb_get_string(const char *name)
{
	const char* result = 0;
	lua_getfield(L, -1, name);
	check(lua_isstring(L, -1), "Lua error, value %s is not a string", name);

	result = lua_tostring(L, -1);

	lua_pop(L, 1);

error:
	return result;
}

int lb_call(lb_func name)
{
	int res;
	int ret = 1;

	check(lb_get_table(TABLENAME) == 0, "%s is not a table", TABLENAME);
	check(name, "name can't be null");

	lua_getfield(L, -1, name);
	check(lua_isfunction(L, -1), "%s isn't a function", name);

	res = lua_pcall(L, 0, 0, 0);
	if (res != 0) {
		log_error("Lua error: %s", lua_tostring(L, -1));
	}

	ret = 0;

error:
	lua_pop(L, 1);
	return ret;
}

lb_func lb_is_callback(const char *name)
{
	lb_func f = NULL;
	if (lb_get_table(TABLENAME) == 1)
		goto error;

	lua_getfield(L, -1, name);

	if (lua_isfunction(L, -1))
		f = name;

	lua_pop(L, 2);

error:
	return f;
}
