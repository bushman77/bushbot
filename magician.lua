local Magician = {}
-- load required modules
local mq = require('mq')
local Common = require('common')

-- Top Level Object mapping
local Me  = mq.TLO.Me
local Pet = mq.TLO.Pet
local Spell = mq.TLO.Spell
local Cast = mq.TLO.Cast
local Spawn = mq.TLO.Spawn
local Target = mq.TLO.Target
local FindItem = mq.TLO.FindItem
spells = {}
spells[1]  = Spell("Spear of Molten Luclinite").ID()
spells[2]  = Spell("Roiling Servant").ID()
spells[3]  = Spell("Unmend the Unnatural").ID()
spells[4]  = Spell("Burnout XV").ID()
spells[5]  = Spell("Aegis of Rumblecrush").ID
spells[6]  = Spell("Gather Potency").ID()
spells[7]  = Spell("Ophiolite Bodyguard").ID()
spells[8]  = Spell("Grant Shak Dathor's Armaments").ID()
spells[9]  = Spell("Volcanic Veil").ID()
spells[10] = Spell("Circle of Emberweave Coat").ID()
spells[11] = Spell("Relentless Guardian").ID()
spells[12] = Spell("Malosinetra").ID()
spells[13] = Spell("Conscription of Earth").ID()

function Magician.pet_buffs()
  if(Me.Class()=="Magician" and Pet.ID()) then
    if(not Me.Aura("Arcane Distillect").ID()) then
      Common.cast('Arcane Distillect', 13, Me.Name())
    end
    Common.buff('Volcanic Veil', 9, Pet())
    Common.buff('Aegis of Rumblecrush', 6, Pet())
  end
end

function Magician.memorize_spells()
  if(Me.Class()=="Magician") then
    for i=1,13 do
      Common.memorize(spell, slot)
    end 
  end
end

function cast_item(item)
  mq.cmdf('/cast item "%s"', item)
end

function Magician.attacks()
  if(Me.Class()=='Magician') then
    print(Me())
    mq.cmd('/assist phrogeater')
    mq.cmd('/attack on')
    mq.cmd('/pet attack')
    if(Me.PctMana()<30) then Common.cast('Gather Potency',6, Me) end
    if(Me.PctMana()<30) then cast('Radiant Modulation Shard',Me) end
    if(not FindItem('Radiant Modulation Shard')) then
       cast_item('Summon Modulation Shard', Me) 
    end
    if(not Me.Pet.Buff('Burnout XV')) then Common.cast('Burnout XV', 4,  Me.Name()) end
    if(Me.SpellReady('Roiling Servant')) then Common.cast('Roiling Servant', 2, Target) end
    if(Me.SpellReady('Spear of Molten Luclinite')) then 
      Common.cast('Spear of Molten Luclinite', 1, Target) 
    end
  end
end
return Magician
