#Advanced Lua


##源代码编译注意事项
本片文档中的源代码都在当前目录下。

lua_c 文件夹下存放的是Lua与C交互的例子源代码。

- lua_c/include 为lua头文件
- lua_c/lib 为Lua静态库，本人是在MacOS上编译的，其他平台需要自行编译。
- 本文中使用的Lua版本为5.1.4

## Lua之metatable


### metatable与访问控制

1. 当访问table中一个不存在的字段时，会调用metatable的__index（调用方法或查找table）。
- 当对table中不存在的索引赋值时，会调用metatable的__newindex（调用方法或查找table）。

根据如上两条性质，可实现下述功能。

#### 具有默认值的table
```
local key = {}
local mt = {__index = function(t) return t[key] end}

function setDefault(t, value)
    t[key] = value
    setmetatable(t, mt)
end

test = {x=5, y=10}
print(test.x, test.y, test.z)
setDefault(test, 11111)
print(test.x, test.y, test.z)
```

#### 跟踪table访问

```
local direct = {x = 5, y = 10}
print (direct.x, direct.y)
direct.z = 20

local indirect = {}
local _indirect = {x = 5, y = 10}
local mt = {
    __index = function(t, k)
        print("access key "..tostring(k))
        return _indirect[k]
    end,
    __newindex = function(t, k, v)
        print ("set key ".. tostring(k) .. " to " .. tostring(v))
        _indirect[k] = v
    end
}
setmetatable(indirect, mt)
print(indirect.x, indirect.y)
indirect.x = 10
indirect.z = 30
```
如上述代码所示，indirect 为对direct的跟踪访问模式。在真实项目中，如果数据在某些地方被修改，而代码量又很大，很难查找时，可以用此种方法来debug.
该代码的输出如下：

```
lizi@lizi:~/workspace/test/lua/advanced$ lua trace.lua 
5	10
access key x
access key y
5	10
set key x to 10
set key z to 30
```

#### 只读table

```
function readOnly(t)
    local proxy = {}
    
    local mt = {
        __index = function(t, k)
            return t[k]
        end,
        __newindex = function(t, k ,v)
            error("change readonly value")
        end
    }
    setmetatable(proxy, mt)
    return proxy
end

local test={a = 10, b = 20}
local readonlyTest = readOnly(t)
readonlyTest.a = 20
```

代码执行结果如下：

```
lizi@lizi:~/workspace/test/lua/advanced$ lua readonly.lua 
lua: readonly.lua:9: change readonly value
stack traceback:
	[C]: in function 'error'
	readonly.lua:9: in function <readonly.lua:8>
	readonly.lua:18: in main chunk
	[C]: ?

```

##metatable与继承
lua中并没有继承，但是从上描述我们知道，当访问table中不存在的字段，会调用metatable的__index。根据此项特性，我们可以模拟继承。

```
local Super = {}
function Super:a()
    print ("Super a")
end

function Super:show()
    print ("Super show")
end

local Sub = {}
local mt = {__index = Super}
setmetatable(Sub, mt)
function Sub:show()
    print ("Sub show")
end

Sub:a()
Sub:show()
```

上述代码是一个简单的继承模拟，Super有两个方法a,show. Sub继承自Super，并且改写了show. 运行结果如下：

```
lizi@lizi:~/workspace/test/lua/advanced$ lua basic_inherit.lua 
Super a
Sub show
```
上述例子简单讲了继承的实现方法，但并不是真正意义上的继承，例如类和实例不分；子类重写了父类的接口，又需要调用父类的接口；构造函数的处理等。所以我们看一个更复杂的例子，具体参考quick-cocos2dx中继承的处理方式。

```
function class(classname, super)
    local superType = type(super)
    local cls

    if superType ~= "function" and superType ~= "table" then
        superType = nil
        super = nil
    end

    if superType == "function" or (super and super.__ctype == 1) then
        -- inherited from native C++ Object
        cls = {}

        if superType == "table" then
            -- copy fields from super
            for k,v in pairs(super) do cls[k] = v end
            cls.__create = super.__create
            cls.super    = super
        else
            cls.__create = super
            cls.ctor = function() end
        end

        cls.__cname = classname
        cls.__ctype = 1

        function cls.new(...)
            local instance = cls.__create(...)
            -- copy fields from class to native object
            for k,v in pairs(cls) do instance[k] = v end
            instance.class = cls
            instance:ctor(...)
            return instance
        end

    else
        -- inherited from Lua Object
        if super then
            cls = {}
            setmetatable(cls, {__index = super})
            cls.super = super
        else
            cls = {ctor = function() end}
        end

        cls.__cname = classname
        cls.__ctype = 2 -- lua
        cls.__index = cls

        function cls.new(...)
            local instance = setmetatable({}, cls)
            instance.class = cls
            instance:ctor(...)
            return instance
        end
    end

    return cls
end

Super = {}
function Super:ctor(...)
    for i, v in pairs(...) do
        self[i] = v
    end
end

function Super:show()
    for i, v in pairs(self) do 
        print(i, v)
    end
end

Sub = class("Sub", Super)

subInstance = Sub.new({a=10, b=20})
subInstance:show()

function Sub:show()
    print("Sub override")
    self.super.show(self)
end
subInstance:show()
```

上述代码中class函数摘自quick-cocos2dx，代码运行结果如下：

```
a	10
class	table: 0x7f8c0ac0b4e0
b	20
Sub override
ctor	function: 0x7f8c0ac0b2c0
show	function: 0x7f8c0ac0b460
lizi@lizi:~/workspace/test/lua/advanced$ vim class.lua 
lizi@lizi:~/workspace/test/lua/advanced$ lua class.lua 
a	10
class	table: 0x7fcf71c089a0
b	20
Sub override
a	10
class	table: 0x7fcf71c089a0
b	20
```

对class函数做一些简单的分析：

- 基类可以是函数或者table，但其他的都不行
- 基类可以是C++对象或者是Lua对象
- 当基类是函数时，子类的new函数会调用基类的函数来创建实例。
- 当基类是C++对象时，子类会拷贝基类中的所有kv到自己上，并且保存基类的__create函数和基类本身。子类的new函数在创建实例时，会调用基类的__create函数，并且将所有kv都拷贝到实例上。
- 若基类是Lua对象，则设置子类的metatable的__index为父类，设置自己的super为父类，并且设置__index为自身。子类的new函数在创建实例时，会设置实例的metatable为子类本身，并且调用实例的ctor函数。

## C与Lua
Lua天生就是一门胶水语言，Lua能很轻松的与C交互。
### 第一个例子
hello.c

```
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

```

hello.lua

```
print("Hello world")

function show()
    print ("global function")
end
```

编译之后运行。结果如下：

```
lizi@lizi:~/workspace/test/lua/advanced/lua_c/first_sample$ ./hello 
Hello world
global function
```

如上，C代码调用Lua流程如下:

1. 创建lua_State
- 注册自己需要的Lua模块，如果默认所有都需要，调用luaL_openlibs.
- 调用C函数加载执行lua代码。
- 可以直接使用luaL_dostring执行已加载的Lua文件中全局函数。

### Lua栈
Lua只用栈在Lua与C之间交互数据，栈里面能保存任何类型的Lua值。
#### 压入元素

```
LUA_API void  (lua_pushnil) (lua_State *L);
LUA_API void  (lua_pushnumber) (lua_State *L, lua_Number n);)
LUA_API void  (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API void  (lua_pushlstring) (lua_State *L, const char *s, size_t l);
LUA_API void  (lua_pushstring) (lua_State *L, const char *s);
LUA_API const char *(lua_pushvfstring) (lua_State *L, const char *fmt,
LUA_API const char *(lua_pushfstring) (lua_State *L, const char *fmt, ...);
LUA_API void  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, int n);
LUA_API void  (lua_pushboolean) (lua_State *L, int b);
LUA_API void  (lua_pushlightuserdata) (lua_State *L, void *p);
LUA_API int   (lua_pushthread) (lua_State *L);
```

#### 查询元素

Api使用索引来引用栈里面的内容，第一入栈的元素索引为1，第N个为N。还可以以栈顶为参考，栈顶为-1，栈顶下面为-2，以此类推。

```
LUA_API int             (lua_isnumber) (lua_State *L, int idx);
LUA_API int             (lua_isstring) (lua_State *L, int idx);
LUA_API int             (lua_iscfunction) (lua_State *L, int idx);
LUA_API int             (lua_isuserdata) (lua_State *L, int idx);
#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)
```

lua_is*判断栈里面的内容是否为某种类型。

```
LUA_API lua_Number      (lua_tonumber) (lua_State *L, int idx);
LUA_API lua_Integer     (lua_tointeger) (lua_State *L, int idx);
LUA_API int             (lua_toboolean) (lua_State *L, int idx);
LUA_API const char     *(lua_tolstring) (lua_State *L, int idx, size_t *len);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, int idx);
LUA_API void	       *(lua_touserdata) (lua_State *L, int idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, int idx);
LUA_API const void     *(lua_topointer) (lua_State *L, int idx);
#define lua_tostring(L,i)	lua_tolstring(L, (i), NULL)
```

lua_to*将栈里面内容转换为某种类型。

#### 其他栈操作

Lua还提供了一些栈的基本操作。

```
LUA_API int   (lua_gettop) (lua_State *L);
LUA_API void  (lua_settop) (lua_State *L, int idx);
LUA_API void  (lua_pushvalue) (lua_State *L, int idx);
LUA_API void  (lua_remove) (lua_State *L, int idx);
LUA_API void  (lua_insert) (lua_State *L, int idx);
LUA_API void  (lua_replace) (lua_State *L, int idx);
LUA_API int   (lua_checkstack) (lua_State *L, int sz);
```

- lua_gettop返回栈中元素的个数。
- lua_settop将栈顶设置为指定位置，如果新栈顶比之前低，则会push nil，反之，会弹出多的元素。
- lua_pushvalue将指定栈中指定位置值的副本压入栈。
- lua_remove删除指定索引上的元素，并将该位置之上的下移。
- lua_insert将指定位置上的元素上移，开辟一个槽位，并将栈顶元素移动到此处。
- lua_replace会弹出栈顶的值，并将该值设置到指定索引，但不会移动任何东西。
- lua_checkstack检查栈中是否有足够空间。

#### C中Lua table操作

```
void lua_getglobal (lua_State *L, const char *name);
void lua_gettable (lua_State *L, int index);
void lua_settable (lua_State *L, int index);
int lua_next (lua_State *L, int index);
```

- 通过lua_getglobal获取lua文件中全局定义的变量，并放置在栈顶
- lua_gettable会从栈顶push key出来，并且讲stack中index位置的table中key对应的值取出，并push到栈顶
- lua_settable设置stack中index位置table中key对应的value。value的位置在栈顶，key的位置为栈顶下一个元素。执行完成后会弹出value和key。
- lua_next从栈顶弹出一个key，并且压入key-value队到栈中。如果table中已经遍历结束，则lua_next返回0。开始遍历时先push nil到栈中。

实例代码如下, 源代码在文件夹lua_c/c_lua_table中。

hello.c

```
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
```

hello.lua

```
test = {a = 5, b = 10, str="Hello world"}
```

运行结果如下:

```
lizi@lizi:~/workspace/test/lua/advanced/lua_c/c_lua_table$ ./hello 
test[a] = 5
key=a value=5
key=add value=1111
key=str value=Hello world
key=b value=10
```

#### C中调用Lua函数
第一个例子中用lua_dostring执行全局函数，本章讲述更常用的调用Lua函数方式。本章中代码在lua_c/c_invoke_lua文件夹中。

hello.c
```
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int main(int argc, char** argv)
{
    lua_State* L =luaL_newstate(); //Create lua vm
    luaL_openlibs(L); //Register standard library

    luaL_dofile(L, "hello.lua");

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

    //invoke with 2 parameters and 1 return value
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
```

hello.lua

```
function dump(t, int_value)
    for i, v in pairs(t) do
        print(i, v)
    end
    print("int value " .. tostring(int_value))
    return "success"
end
```

程序输出结果为：

```
lizi@lizi:~/workspace/test/lua/advanced/lua_c/c_invoke_lua$ ./hello 
y	hello
x	10
int value 50
call return success
```

#### Lua中调用C函数

Lua调用C函数步骤如下：

1. 编写C函数，函数参数为luaState*.
- C函数接收的Lua参数从栈中获取，返回Lua的参数也push在栈中，返回值为回传给Lua参数个数。
- 使用luaL_register将C函数注册给lua_State.
- lua中require "name"时，会调用luaopen_name注册C库中的函数。

代码在lua_c/lua_invoke_c中。
hello.c,编译成hello.so

```
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
```

test.lua

```
require "hello"
print(hello.add(10, 20))
```

编译成so后，在当前目录下执行lua test.lua,结果如下：

```
lizi@lizi:~/workspace/test/lua/advanced/lua_c/lua_invoke_c$ lua test.lua 
30
```

上述例子是直接从Lua解释器里面调用C代码，再举一个更复杂一点的例子,从C调用Lua，Lua里面又调用C里面的函数。代码目录为lua_c/c_lua_c.

hello.c

```
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
```

test.lua

```
function dump(t, int_value)
    -- invoke c function
    t.c_value = hello.add(t.x,  int_value)

    for i, v in pairs(t) do
        print(i, v)
    end
    print("int value " .. tostring(int_value))
    return "success"
end
```

执行结果如下:

```
y	hello
x	10
c_value	60
int value 50
call return success
```

上述例子中，我们新增了一个自己的模块，在main函数中创建lua_State后将自己的模块注册进lua_State。所以在后续的lua文件中，可以不require而直接使用该模块。

## #TODO
因为时间有限，所以暂时只写了两个模块。后续有时间还会讲解Lua UserData, Lua Coroutine， Lua弱引用以及GC。










