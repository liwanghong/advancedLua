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
