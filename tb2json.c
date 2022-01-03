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

int main(argsc, args, env)
int argsc;
char **args, **env;
{
    lua_State *s = luaL_newstate();


    lua_newtable(s);

    for (int i = 1; i < 9; i++, lua_settable(s, -3)) {
	lua_pushnumber(s, i);
	lua_newtable(s);
	lua_pushnumber(s, 1);
	{
	    lua_newtable(s);
	    lua_newtable(s);
	    for (int k = 1; k < 90; k++) {
		lua_pushnumber(s, k);
		lua_pushnumber(s, k * 3);
		lua_settable(s, -3);
	    }
	    lua_setfield(s, -2, "Kk");
	}
	lua_settable(s, -3);
    }

    json_t *ja = tbConv(s, 1);
    printf("%d\n", lua_gettop(s));

    lua_close(s);
    return 0;
}
