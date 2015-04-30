#include "luabinding.h"

#include "util.h"
#include "global.h"

/* Window function bindings */
static int lb_window_test(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	pdebug("Called from lua, by: %p window id: %d", client, client->window);

	return 0;
}

static int lb_window_move(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	int x, y;
	check(client, "client is null");

	x = luaL_checkint(l, 2);
	y = luaL_checkint(l, 3);

	move_window(c, client, x, y);
	xcb_flush(c);

error:
	return 0;
}

static int lb_window_move_relative(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	int x, y;
	check(client, "client is null");

	x = luaL_checkint(l, 2);
	y = luaL_checkint(l, 3);

	move_window_relative(c, client, x, y);
	xcb_flush(c);

error:
	return 0;
}

static int lb_window_resize(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	int w, h;
	check(client, "client is null");

	w = luaL_checkint(l, 2);
	h = luaL_checkint(l, 3);

	resize_window(c, client,
		w - client->border_width * 2,
		h - client->border_width * 2);

	xcb_flush(c);

error:
	return 0;
}

static int lb_window_map(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	check(client, "client is null");

	map_window(c, client);
	xcb_flush(c);

error:
	return 0;
}

static int lb_window_unmap(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	check(client, "client is null");

	unmap_window(c, client);
	xcb_flush(c);
error:
	return 0;
}

static int lb_window_focus(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	check(client, "client is null");

	focus_window(c, client, &focused);
	xcb_flush(c);

error:
	return 0;
}

static int lb_window_getx(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	check(client, "client is null");
	lua_pushnumber(l, client->x);
error:
	return 1;
}

static int lb_window_gety(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	check(client, "client is null");
	lua_pushnumber(l, client->y);
error:
	return 1;
}

static int lb_window_getw(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	check(client, "client is null");
	lua_pushnumber(l, client->width + client->border_width * 2);
error:
	return 1;
}

static int lb_window_geth(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	check(client, "client is null");
	lua_pushnumber(l, client->height + client->border_width * 2);
error:
	return 1;
}

static int lb_window_mapped(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	check(client, "client is null");
	lua_pushboolean(l, client->mapped);
error:
	return 1;
}

static int lb_window_hasfocus(lua_State *l) {
	client_t *client = lua_touserdata(l, 1);
	check(client, "client is null");
	lua_pushboolean(l, (client == focused));
error:
	return 1;
}

static const struct luaL_Reg windowlib_m[] = {
	{ "test", lb_window_test },
	{ "move", lb_window_move },
	{ "move_relative", lb_window_move_relative },
	{ "resize", lb_window_resize },
	{ "show", lb_window_map },
	{ "hide", lb_window_unmap },
	{ "focus", lb_window_focus },
	{ "getx", lb_window_getx },
	{ "gety", lb_window_gety },
	{ "getw", lb_window_getw },
	{ "geth", lb_window_geth },
	{ "mapped", lb_window_mapped },
	{ "hasfocus", lb_window_hasfocus },
	{ NULL, NULL }
};

int keyindex = 0;

static int lb_key_bind(lua_State *l)
{
	int i;
	static int cb_index = 1;

	// can't have old data
	memset(keys + keyindex, 0, sizeof(keypress_t));

	keys[keyindex].name = luaL_checkstring(l, 1);

	if (lua_istable(l, 2)) {
		for (i = 1; i < 5; i++) {
			lua_pushnumber(l, i);
			lua_gettable(l, 2);
			const char* mod = lua_tostring(l, -1);
			if (!mod) break;

			if (strcmp(mod, "mod4") == 0)
				keys[keyindex].mask |= XCB_MOD_MASK_4;
			else if (strcmp(mod, "ctrl") == 0)
				keys[keyindex].mask |= XCB_MOD_MASK_CONTROL;
			else if (strcmp(mod, "alt") == 0)
				keys[keyindex].mask |= XCB_MOD_MASK_1;
			else if (strcmp(mod, "shift") == 0)
				keys[keyindex].mask |= XCB_MOD_MASK_SHIFT;
		}
	}

	if (lua_isfunction(l, 3)) {
		lb_get_table(TABLENAME);
		lua_getfield(l, -1, "key_callbacks");

		lua_pushvalue(l, 3);
		lua_rawseti(l, -2, cb_index);

		keys[keyindex].callback_index = cb_index;

		cb_index++;
		lua_pop(l, 2);
	}

	keyindex++;

	return 0;
}

int lb_get_keycount()
{
	return keyindex;
}

void lb_on_keypress(int index)
{
	lb_get_table(TABLENAME);
	lua_getfield(L, -1, "key_callbacks");
	lua_rawgeti(L, -1, index);
	lua_pcall(L, 0, 0, 0);

	lua_pop(L, 2);
}

void lb_init(int w, int h)
{
	L = luaL_newstate();
	check(L, "Failed to get new lua state");
	luaL_openlibs(L);

	/* Create the kass table in lua */
	lua_newtable(L);

	lua_newtable(L);
	lua_setfield(L, -2, "clients");

	lua_newtable(L);

	lua_pushnumber(L, w);
	lua_setfield(L, -2, "width");
	lua_pushnumber(L, h);
	lua_setfield(L, -2, "height");

	lua_setfield(L, -2, "screen");

	lua_newtable(L);
	lua_setfield(L, -2, "key_callbacks");

	lua_newtable(L);
	lua_pushcfunction(L, lb_key_bind);
	lua_setfield(L, -2, "bind");
	lua_setfield(L, -2, "key");

	/* Metatable for windows */
	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, windowlib_m, 0);
	lua_setfield(L, -2, "window");

	lua_setglobal(L, TABLENAME);

error:
	/* die here? */
	return;
}

void lb_load_config(const char *filename, CONFIG *cfg)
{
	if (luaL_dofile(L, filename)) {
		log_warn("error loading config file %s", filename);
		exit(1);
	}

	lb_get_table("config");

	cfg->border_width = lb_get_number("border_width");
	cfg->border_normal_color = lb_get_number("border_normal_color");
}

void lb_cleanup()
{
	lua_close(L);
	keyindex = 0;
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

void lb_new_window(client_t **client, xcb_window_t window)
{
	*client = lua_newuserdata(L, sizeof(client_t));
	new_window(c, *client, window);

	/* This is dumb but i can't use luaL_setmetatable(L, "kass.window")
	 * for some reason*/
	lb_get_table(TABLENAME);
	lua_getfield(L, -1, "window");
	lua_setmetatable(L, -3);

	/* Add to kass.clients */
	lua_getfield(L, -1, "clients");
	check(lua_istable(L, -1), "clients field isn't a table, can't add window");

	lua_pushnumber(L, (*client)->window);
	lua_pushvalue(L, -4);

	lua_settable(L, -3);

error:
	lua_pop(L, 2);
}

void lb_remove_window(client_t *client)
{
	if (lb_get_table(TABLENAME)) {
		log_error("Failed to get clients table");
	}
	
	if (!client)
		goto error; // Die silently

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

void lb_push_number(int number)
{
	lua_pushnumber(L, number);
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

