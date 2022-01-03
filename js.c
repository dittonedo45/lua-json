#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <jansson.h>
static void _Ladd_js(lua_State * s, json_t * t)
{
    if (!t) {
	lua_pushnil(s);
	return;
    }
    switch (json_typeof(t)) {
    case JSON_TRUE:
	lua_pushboolean(s, 1);
	break;
    case JSON_FALSE:
	lua_pushboolean(s, 0);
    case JSON_NULL:
	lua_pushnil(s);
	break;
    case JSON_INTEGER:
    case JSON_REAL:
	lua_pushnumber(s, json_number_value(t));
	break;
    case JSON_STRING:
	lua_pushstring(s, json_string_value(t));
	break;
    case JSON_OBJECT:
	{
	    lua_newtable(s);
	    const char *k;
	    json_t *tv;

	    json_object_foreach(t, k, tv)
		// Bound
	    {
		_Ladd_js(s, tv);
		lua_setfield(s, -2, k);
	    }
	}
	break;
    case JSON_ARRAY:
	{
	    lua_newtable(s);
	    int i;
	    json_t *j;

	    json_array_foreach(t, i, j)
		// Bound
	    {
		lua_pushnumber(s, i + 1);
		_Ladd_js(s, j);
		lua_settable(s, -3);
	    }
	}
    }

}

static void Ladd_js(lua_State * s, char *b)
{
    json_t *job = json_loads(b, 0, 0);
    _Ladd_js(s, job);
}

static int xx_tojson(lua_State * s)
{
    const char *str = luaL_checkstring(s, 1);
    Ladd_js(s, str);
    return 1;
}

int main(argsc, args, env)
int argsc;
char **args, **env;
{
    lua_State *s = luaL_newstate();
    lua_pushcfunction(s, xx_tojson);
    lua_setglobal(s, "json2table");

    luaL_openlibs(s);
    luaL_loadfile(s, args[1]);
    lua_call(s, 0, 0);
    lua_settop(s, 0);
    lua_newtable(s);
    lua_pushcfunction(s, xx_tojson);
    lua_setfield(s, -2, "nerd");

    lua_pushnil(s);
    int type = lua_next(s, -2);
    printf("%s -> []\n", lua_tostring(s, -2));
    lua_close(s);
    return 0;
}
