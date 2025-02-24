local Cleric = {}
-- load required modules
local mq = require('mq')
local Common = require('common')

-- Top Level Object mapping
local Cast = mq.TLO.Cast
local Group = mq.TLO.Group
local Me  = mq.TLO.Me
local Pet = mq.TLO.Pet
local Spell = mq.TLO.Spell
local Spawn = mq.TLO.Spawn
local Target = mq.TLO.Target

spells = {}
spells[1] = Spell("Avowed Remedy Rk. II").ID()
spells[2] = Spell("Guileless Remedy").ID()
spells[3] = Spell("Promised Remediation Rk. II").ID()
spells[4] = Spell("Avowed Intervention Rk. II").ID()
spells[5] = Spell("Atoned Intervention").ID()
spells[6] = Spell("Rallied Greater Aegis of Vie Rk. II").ID()
spells[7] = Spell("Elixir of Realization").ID()
spells[8] = Spell("Word of Greater Vivification Rk. II").ID()
spells[9] = Spell("Syllable of Acceptance Rk. II").ID()
spells[10] = Spell("Unified Hand of Infallibility").ID()
spells[11] = Spell("Nineteenth Commandment Rk. II").ID()
spells[12] = Spell("Shining Steel Rk. II").ID()
spells[13] = Spell("Ward of Commitment Rk. II").ID()

function Cleric.memorize_spells()
  if(Me.Class()=="Cleric") then
    for i=1,13 do
      Common.memorize(spells[i], i)
    end 
  end
end

-- private function that takes index and maps it to group.Member to return string format for that character
local function char(indx) return Group.Member(indx) end

-- this function takes 4 arguments, groupmember index int, spell string, opr string and percentage int
local function heal(i, spell, slot, opr, pct)
  -- thank indx and map it to group member and store it in varliable as string
  if(opr == '<=') then
    if(char(i).CurrentHPs() <= pct and Me.SpellReady(spell)) then 
      Common.cast(spell, slot, char(i)) 
    end
  end
end

function Cleric.heals()
  if(Me.Class() == "Cleric") then
      --mq.cmd('/dgt all I am suppose to be healing i dontknow why im not')
    if(not Me.Song("Elixir of Realization").ID()) then 
      -- and Spawn("Phrogeater").Distance() < 50) then
      Common.cast("Elixir of Realization", 7, Me()) 
    end
    for i=0,Group() do
      heal(Group.Member(i), 'Avowed Remedy Rk. II', 1, '<=', 75)
      heal(Group.Member(i), 'Guileless Remedy', '<=', 2, 75)
      heal(Group.Member(i), 'Avowed Intervention Rk. II', 4, '<=', 99)
      heal(Group.Member(i), 'Atoned Intervention', 5, '<=', 99)
      mq.delay(1)
    end
  end

end
return Cleric
