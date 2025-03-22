function outer()
    local x = 10
    function inner()
        print(x)
    end
    return inner
end