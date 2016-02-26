#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static int add(lua_State* L)
{
    if (lua_gettop(L) != 2)
    {
        return luaL_error(L, "invalid parameter number");
    }

    if (lua_isnumber(L, -1) == 0)
    {
        return luaL_error(L, "invalid parameter type");
    }

    if (lua_isnumber(L, -2) == 0)
    {
        return luaL_error(L, "invalid parameter type");
    }

    int x = lua_tonumber(L, -1);
    int y = lua_tonumber(L, -2);

    lua_pushinteger(L, x+y);
    return 1;
}

static const struct luaL_Reg mylib[] = {
    {"add", add},
    {NULL, NULL}
};

int luaopen_hello(lua_State *L) 
{
    luaL_register(L, "hello", mylib);
    return 1;
}
