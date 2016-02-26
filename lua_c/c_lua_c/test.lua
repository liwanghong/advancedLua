function dump(t, int_value)
    -- invoke c function
    t.c_value = hello.add(t.x,  int_value)

    for i, v in pairs(t) do
        print(i, v)
    end
    print("int value " .. tostring(int_value))
    return "success"
end

