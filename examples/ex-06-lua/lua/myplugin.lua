-- ex-06-lua: lua/myplugin.lua
require('mylib')
local timer

function restart(r)
   c8y:send('105,' .. r:value(2) .. ',EXECUTING')
   for i = 0, r.size - 1 do     -- index in C++ starts from 0.
      srDebug(r:value(i))
   end
   c8y:send('105,' .. r:value(2) .. ',SUCCESSFUL')
end

function cpuMeasurement()
   local cpu = math.random(100)
   c8y:send('103,' .. c8y.ID .. ',' .. cpu)
end

function init()
   srDebug(myString)            -- myString from mylib
   timer = c8y:addTimer(10 * 1000, 'cpuMeasurement')
   c8y:addMsgHandler(502, 'restart')
   timer:start()
   return 0                     -- signify successful initialization
end
