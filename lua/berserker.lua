local Berserker = {}
local mq = require('mq')
local Me = mq.TLO.Me

function Berserker.attacks()
  if(Me.Class() == "Berserker") then
    mq.cmd('/assist phrogeater')
    mq.cmd('/attack on')
    if(Me.AltAbilityReady("Battle Leap")) then mq.cmd('/alt activate 611') end
  end
end

function Berserker.Buff()

end

return Berserker
