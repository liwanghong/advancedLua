#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int main(int argc, char** argv)
{
    lua_State* L =luaL_newstate(); //Create lua vm
    luaL_openlibs(L); //Register standard library

    luaL_dofile(L, "hello.lua");

    lua_getglobal(L, "test");

    lua_pushstring(L, "a");
    lua_gettable(L, -2); //Now stack top is test[a]

    if (lua_isnumber(L, -1) == 0)
    {
        printf("wrong data type\n");
        return 1;
    }

    int value = lua_tointeger(L, -1);
    printf("test[a] = %d\n", value);
    lua_pop(L, 1); //pop value, stack top is table test

    lua_pushstring(L, "add");
    lua_pushinteger(L, 1111);
    lua_settable(L, -3);

    //start to iterator test
    lua_pushnil(L);
    while(lua_next(L, -2) != 0)
    {
        //get key
        const char* key = lua_tostring(L, -2);
        if (lua_isnumber(L, -1) == 1)
        {
            int iter_value = lua_tointeger(L, -1); 
            printf("key=%s value=%d\n", key, iter_value);
        }
        else if (lua_isstring(L, -1) == 1)
        {
            const char* iter_value = lua_tostring(L, -1);
            printf("key=%s value=%s\n", key, iter_value);
        }

        lua_pop(L, 1); //pop value and keep key for next iterator
    }
    
    return 0;
}
