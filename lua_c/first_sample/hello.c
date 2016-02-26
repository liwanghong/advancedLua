#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int main(int argc, char** argv)
{

    lua_State* L =luaL_newstate(); //Create lua vm
    luaL_openlibs(L); //Register standard library

    luaL_dofile(L, "hello.lua");

    luaL_dostring(L, "show()");

    return 0;
}
