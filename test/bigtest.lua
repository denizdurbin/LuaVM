-- Global variable test (SETGLOBAL, GETGLOBAL)
x = 10
y = 20
z = "hello"

-- Arithmetic operations (ADD, SUB, MUL, DIV, MOD, POW, UNM)
sum = x + y      -- ADD
diff = x - y     -- SUB
prod = x * y     -- MUL
quot = x / y     -- DIV
remainder = x % 3 -- MOD
power = x ^ 2    -- POW
neg = -x         -- UNM

-- Boolean operations (NOT)
flag = true
notFlag = not flag  -- NOT

-- String operations (LEN, CONCAT)
str1 = "Lua"
str2 = "VM"
concatenated = str1 .. " " .. str2  -- CONCAT
lenStr = #concatenated  -- LEN


-- Boolean assignment test (LOADBOOL)
isPositive = true
isNegative = false

-- Nil assignment (LOADNIL)
unsetVar = nil  -- LOADNIL

-- Printing results (CALL, involving a built-in function)
print("Sum:", sum)
print("Difference:", diff)
print("Product:", prod)
print("Quotient:", quot)
print("Remainder:", remainder)
print("Power:", power)
print("Negation:", neg)
print("Concatenation:", concatenated)
print("String Length:", lenStr)
print("Is Positive:", isPositive)
print("Is Negative:", isNegative)
print("Unset Var:", unsetVar)
print("Flag:", flag)
print("Not Flag:", notFlag)
