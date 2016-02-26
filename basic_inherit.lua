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

