local timer

function init()
   timer = c8y:addTimer(10*1000, 'sendMeasurement')
   timer:start()
   c8y:addMsgHandler(803, 'handleRelay')
   return 0
end


function sendMeasurement()
   c8y:send('305,' .. c8y.ID .. ',-79,3' )
end


function handleRelay(r)
   if r.size >= 4 then
      c8y:send('303,' .. r:value(2) .. ',EXECUTING')
      if r:value(3) == 'OPEN' then
         c8y:send('304,' .. r:value(2) .. ',"Do not do it!"')
      elseif r:value(3) == 'CLOSED' then
         c8y:send('303,' .. r:value(2) .. ',SUCCESSFUL')
      else
         c8y:send('304,' .. r:value(2) .. ',"Unknown operation!"')
      end
   end
end
