local Beastlord = {}
local mq = require('mq')
local Me = mq.TLO.Me

function Beastlord.attacks()
  if(Me.Class() == "phrogeater") then
    mq.cmd('/assist phrogeater')
    mq.cmd('/attack on')
    mq.cmd('/pet attack on')
  end
end

return Beastlord
