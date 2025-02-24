local Common = {} -- This is bascially the Module Name
-- Load up required modules
local mq     = require('mq')
local Me     = mq.TLO.Me
local Spell  = mq.TLO.Spell
local Target = mq.TLO.Target

function Common.hello()
  print("world")
end

-- Private Functions, these are here to be only accessable only to the parent module

-- Public Function
function Common.buff(spell, slot, target)
  mq.cmdf('/target %s', target)
  if(Me.SpellReady(spell) and not Target.Buff(spell).ID()) then 
    mq.cmdf('/casting "%s" %s', spell, slot)
  end
end

function Common.cast_aa(aa, character)
  if(Me.AbilityReady(aa)) then
    mq.cmdf('/target %s', character)
    mq.cmdf('/alt activate %s', aa) 
  end
end

function Common.cast(spell, slot, character)
  if(Me.SpellReady(spell)) then
    mq.cmdf('/target %s', character)
    mq.cmdf('/casting "%s" %s',spell, slot)
    while(Me.Casting()) do mq.delay(1) end
  end
end
function Common.memorize(spell, slot) 
  if Me.Gem(slot).Name() ~= Spell(spell).Name() then
    printf('Memorize %s in slot %s', Spell(spell).Name(), slot)
    mq.cmdf('/memorize "%s" %s', Spell(spell).Name(), slot) 
    mq.delay(5000)
  end
end

return Common
