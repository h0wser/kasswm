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
	lua_newtable(L);
	lua_setfield(L, -2, "clients");
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

void lb_create_window(client_t *client)
{
	lua_newtable(L);

	/* Assign member variables */
	/* push values in reverse order */
	lua_pushboolean(L, client->mapped);
	lua_pushnumber(L, client->height);
	lua_pushnumber(L, client->width);
	lua_pushnumber(L, client->y);
	lua_pushnumber(L, client->x);
	lua_pushnumber(L, client->window);

	/* Assign them in normal order */
	lua_setfield(L, -7, "id");
	lua_setfield(L, -6, "x");
	lua_setfield(L, -5, "y");
	lua_setfield(L, -4, "w");
	lua_setfield(L, -3, "h");
	lua_setfield(L, -2, "mapped");
}

void lb_add_window(client_t *client)
{
	if (lb_get_table(TABLENAME)) {
		log_error("Failed to get clients table");
	}

	lua_getfield(L, -1, "clients");
	check(lua_istable(L, -1), "clients field isn't a table, can't add window");

	lua_pushnumber(L, client->window);
	lb_create_window(client);

	lua_settable(L, -3);

error:
	lua_pop(L, 1); // pop kass table from stack
	return;
}

void lb_remove_window(client_t *client)
{
	if (lb_get_table(TABLENAME)) {
		log_error("Failed to get clients table");
	}

	lua_getfield(L, -1, "clients");
	check(lua_istable(L, -1), "clients field isn't a table, can't add window");

	lua_pushnumber(L, client->window);
	lua_pushnil(L);
	lua_settable(L, -3);

error:
	lua_pop(L, 1); // pop kass table from stack
	return;
}

int lb_push_func(lb_func name)
{
	int ret = 1;
	check(lb_get_table(TABLENAME) == 0, "%s is not a table", TABLENAME);
	check(name, "name can't be null");

	lua_getfield(L, -1, name);
	check(lua_isfunction(L, -1), "%s isn't a function", name);

	ret = 0;
error:

	return ret;
}

int lb_call(int nargs)
{
	int res;
	int ret = 1;

	res = lua_pcall(L, nargs, 0, 0);
	if (res != 0) {
		log_error("Lua error: %s", lua_tostring(L, -1));
	}

	ret = 0;

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
