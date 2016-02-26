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

