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

    int x = lua_tonumber(L, -2);
    int y = lua_tonumber(L, -1);

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

int main(int argc, char** argv)
{
    lua_State* L =luaL_newstate(); //Create lua vm
    luaL_openlibs(L); //Register standard library

    luaopen_hello(L); //Register my module 

    luaL_dofile(L, "test.lua");
    

    lua_getglobal(L, "dump");

    if (lua_isfunction(L, -1) == 0)
    {
        printf("wrong type\n");
        return 1;
    }

    lua_newtable(L);

    //t[x] = 10;
    lua_pushstring(L, "x");
    lua_pushinteger(L, 10);
    lua_settable(L, -3);

    //t[y] = hello
    lua_pushstring(L, "y");
    lua_pushstring(L, "hello");
    lua_settable(L, -3);
   
    lua_pushinteger(L, 50);

    //invoke with 2 paramters and 1 return value
    // stack layout
    // -1  parameter -- interger 50 
    // -2  parameter -- table {x=10, y="hello"}
    // -3  function dump
    if(lua_pcall(L, 2, 1, 0) != 0)
    {
        printf("call error %s\n", lua_tostring(L, -1));
        return 1;
    }
    
    printf("call return %s\n", lua_tostring(L, -1));

    return 0;
}
