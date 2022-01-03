#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <jansson.h>

static json_t *tbConv(lua_State * s, int l)
{
    int type = lua_type(s, l);
    int go = 0;

    json_t *ja = 0;

    switch (type) {
    case LUA_TTHREAD:
	ja = json_string("<Thread>");
	break;
    case LUA_TFUNCTION:
	ja = json_string("<FUNC>");
	break;
    case LUA_TLIGHTUSERDATA:
	ja = json_string("<USERD>");
	break;
    case LUA_TUSERDATA:
	ja = json_string("<Obj>");
	break;
    case LUA_TSTRING:
	ja = json_string(lua_tostring(s, l));
	break;
    case LUA_TNUMBER:
	ja = json_integer(lua_tonumber(s, l));
	break;
    case LUA_TNIL:
    case LUA_TNONE:
	ja = json_null();
	break;
    case LUA_TTABLE:
	if (1) {
	    lua_pushnil(s);
	    go = lua_next(s, l);
	    if (!go) {
		// Assuming Its An Array Chunk
		return json_array();
	    }
	    do {
		if (LUA_TNUMBER == lua_type(s, -2)) {
		    if (!ja)
			ja = json_array();
		    json_array_append(ja, tbConv(s, lua_gettop(s)));
		} else if (LUA_TSTRING == lua_type(s, -2)) {
		    if (!ja)
			ja = json_object();
		    json_object_set(ja, lua_tostring(s, -2),
				    tbConv(s, lua_gettop(s)));
		}
		lua_settop(s, -2);
		go = lua_next(s, l);
		if (!go)
		    break;
	    } while (go);
	    if (!ja)
		ja = json_array();
	}
	break;
    }
    return ja;
}

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
    Ladd_js(s, (char *) str);
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

    char **p = env;

    while (p && *p) {
	char *val = strchr(*p, '=');
	char *key = 0;
	asprintf(&key, "%.*s", val - (*p), *p);
	lua_pushstring(s, val + 1);
	lua_setfield(s, -2, key);
	free(key);

	p++;
    }

    json_t *ja = tbConv(s, lua_gettop(s));

    _Ladd_js(s, ja);

    lua_getfield(s, -1, "HOME");

    puts(lua_tostring(s, -1));

    lua_close(s);
    return 0;
}
