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

