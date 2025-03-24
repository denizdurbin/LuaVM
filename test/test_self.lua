local obj = {
    name = "Test Object",
    greet = function(self, message)
      print("Self name:", self.name)
      return "Hello, " .. message .. " from " .. self.name
    end
  }
  
  local result = obj:greet("World")
  print(result)