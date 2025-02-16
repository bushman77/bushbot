local mq = require('mq')
local actors = require('actors')
local ImGui = require('ImGui')
local dannet = require('lib/dannet/helpers')

-- imports
--local Cleric = require('Cleric')
-- references
-- https://github.com/casssoft/imgui_lua_bindings
-- https://lemonate.io/docs/en/scripting/reference/imgui.html
-- capture global arguments and map them to table
local args = {...}
-- assign each element of global table to a human readable variable
local role = args[1]
local focus = "" 
-- TLO mappedImGuiTableColumnFlags.WidthFixed, 10, 1
-- Create specific NameSpaces for each Top Level Objects
local Me     = mq.TLO.Me
local Target = mq.TLO.Target
local Spawn  = mq.TLO.Spawn
local Group  = mq.TLO.Group
local Spell  = mq.TLO.Spell
local FindItem = mq.TLO.FindItem

local terminate = false
local isOpen, shouldDraw = true, true
local uiinit = 0

-- Assign group members to list
local characters = {}
characters[1] = "Phrogeater"
characters[2] = "Bushman"
characters[3] = "Sandayar"
characters[4] = "Solranacougar"
characters[5] = "Zexerious"
characters[6] = "Skullzsmasher"

local mybuffs = {
    CLR={'Commitment', 'Shining Steel'},
    BST={"Spirit of Tala'Tak", 'Celerity', 'Shared Merciless Ferocity'},
    MAG={'Circle of Emberweave Coat', 'Volcanic Veil', 'Group Perfected Levitation'}
}
mybuffs = mybuffs[mq.TLO.Me.Class.ShortName()]

-- ----------------
-- Public Functions
-- ----------------
local function cast(spell, character)
   if(Me.SpellReady(spell) and Spawn(character).ID() > 0) then 
     mq.cmdf('/multiline ; /target %s; /casting "%s"', character, spell)
     while(Me.Casting()) do end
   end
end

local function berserker_attacks()
  if(Me.Class() == "Berserker") then
    mq.cmd('/assist phrogeater')
    mq.cmd('/attack on')
  end
end

local function cleric_heals()
  if(Me.Class() == "Cleric") then
    for i,character in pairs(characters) do
      -- combat buffs
      if(not Me.Song("Elixir of Realization").ID() and Spawn("Phrogeater").Distance() < 50) then
        cast("Elixir of Realization", character) 
      end
      -- regular spot heals
      if(Spawn(character).CurrentHPs() <= 75) then
        cast("Avowed Remedy Rk. II", character)
        if(Spawn(character).CurrentHPs() >= 90) then break end
        cast("Guileless Remedy", character)
        if(Spawn(character).CurrentHPs() >= 90) then break end
        cast("Avowed Intervention Rk. II", character)
        if(Spawn(character).CurrentHPs() >= 90) then break end
      end
    end
  end
end

local function magician_attacks()
  if(Me.Class() == 'Magician') then
    mq.cmd('/assist phrogeater')
    mq.cmd('/pet attack')
    if(Me.PctMana()<30) then cast('Gather Potency', Me) end
    if(Me.PctMana()<30) then cast('Radiant Modulation Shard',Me) end
    if(not FindItem('Radiant Modulation Shard')) then cast('Summon Modulation Shard', Me) end
    if(not Me.Pet.Buff('Burnout XV')) then cast('Burnout XV', Me.Name()) end
    cast('Roiling Servant', Target)
    cast('Spear of Molten Luclinite', Target)
  end
end

-- ----------------
-- new row function
-- ----------------
local function table_header()
  -- ImGui.TableNextColumn()
  ImGui.TableSetupColumn("Characters", ImGuiTableColumnFlags.WidthFixed, 0, 0, 0) 
  ImGui.TableNextColumn()

  ImGui.TableSetupColumn("Controls", ImGuiTableColumnFlags.WidthFixed, 50, 1)
  ImGui.TableNextColumn()
  ImGui.TableHeadersRow(0,0)
  ImGui.TableNextColumn()        

end
local function group(data)
  for i= 1,6 do 
    if(ImGui.Button(data[i])) then
       focus = data[i]
    end
  end
  --ImGui.TableNextRow()
end
local function controls()
  ImGui.TableNextColumn()
  ImGui.TableNextRow()
end
-- ----------
-- MASTER GUI
-- ----------
local function updateImGui()
  -- Only draw the window contents if shouldDraw is true
  
  --if shouldDraw then
  ImGui.Begin('Bushbot - Master v0.1', true)
  if ImGui.Button('X') then terminate = true end

    -- BeginTable takes 3 arguments: name, number of columns and border width
    ImGui.BeginTable("interface", 2, ImGui.TableFlags_Borders)
      --table_header() 
      ImGui.TableSetupColumn("Characters", ImGuiTableColumnFlags.WidthFixed, 0, 0, 0) 
      ImGui.TableHeadersRow(0,0)
      ImGui.TableNextColumn()
      group(characters)
      ImGui.TableNextColumn()
      --controls()
      if(focus == "") then
        ImGui.Text("No character selected")
      elseif(focus == 'Sandayar') then
        if(ImGui.Button('Call Hero')) then
          mq.cmdf('/dex sandayar /targ %s', Target)
          mq.cmd('/dex sandayar /casting "Call of the Hero"')
        end
      elseif(focus == 'Bushman') then
        if(ImGui.Button('SIT')) then mq.cmdf('/dex %s /sit on', focus) end
        if(ImGui.Button('PACIFY')) then 
          mq.cmdf('/dex bushman /target %s', Target)
          mq.cmd('/dex bushman //casting ""')
          mq.cmdf('/dex %s /sit on', focus) 
        end
      else
        if(ImGui.Button('SIT')) then mq.cmdf('/dex %s /sit on', focus) end
      end
    ImGui.EndTable()
    -- Always call ImGui.End if begin was called
  ImGui.End()  
end

-- ----------------------
-- pure functions
-- ----------------------
local function length(array)
  -- initlize count variable
  count = 0
  -- iteratorate through the list
  for k,v in pairs(array) do
    -- add 1 to the count varable
    count = count + 1
  end
  -- respond with the updated count value returned from for loop
  return count
end

-- actors
-- some example buffs, for demonstration purposes
-- due to BST crashing evertime Spirit of Tala'Tak is cast because it contains a special character in its name the int format is used instead 50228
local buff_queue = {}
local function dobuffs()
    for name, ff in pairs(buff_queue) do
      if buff=='Commitment' then cast('Unified Hand of Infallibility', name) end
      if buff=='Shining Steel' then cast('Shining Steel Rk. II', name) end
      if buff=="Spirit of Tala'Tak" then cast(Spell(buff).ID(), name) end
      if buff=='Circle of Emberweave Coat' then cast(buff, name) end
      if buff=='Celerity' then cast(buff, name) end
      if buff=='Volcanic Veil' then cast(buff, name) end
      if buff=='Shared Merciless Ferocity' then cast(buff, name) end
      if buff=='Group Perfected Levitation' then cast('Perfected Levitation', name) end
    end

    buff_queue = {}
end


-- store a list of buffs and who can cast them
local buffers = {}
local function addbuffer(buff, sender)
    --printf('Received buffer %s casting %s', sender.character, buff)
    if buff and sender then
        if not buffers[buff] then buffers[buff] = {} end

        if not buffers[buff][sender] then
            buffers[buff][sender] = true
        end
    end
end

-- whenever a buffer disconnects, handle that
local function removebuffer(sender)
    for buff, _ in pairs(buffers) do
        buffers[buff][sender] = nil
    end
end

-- this is then message handler, so handle all messages we expect
-- we are guaranteed that the only messages here we receive are
-- ones that we send, so assume the structure of the message
local actor = actors.register(function (message)
    if message.content.id == 'buffs' and message.sender and mybuffs then
        -- request to send a list of buffs I can cast
        for _, buff in ipairs(mybuffs) do
            message:send({id='announce', buff=buff })
        end
    elseif message.content.id == 'beg' then
        -- request for a buff, send back a reply to indicate we are a valid buffer
        message:reply(0, {})
        buff_queue[message.sender.character] = message.content.buff
    elseif message.content.id == 'announce' then
        -- a buffer has announced themselves, add them to the list
        addbuffer(message.content.buff, message.sender)
    elseif message.content.id == 'drop' then
        -- a buffer has dropped, remove them from the list
        removebuffer(message.sender)
    end
end)

-- buffer login, notify all beggars of available buffs
local function bufferlogin()
    for _, buff in ipairs(mybuffs) do
        -- need to specify the actor here because we're sending to beggars
        -- from the buffer actor but leave it loose so that _all_ beggars
        -- receive this message
        --printf('Registering %s on beggars', buff)
        actor:send({id='announce', buff=buff})
    end
end

-- beggar login, request buffer buffs
local function beggarlogin()
    actor:send({id='buffs'})
end

-- beggar buff request, choose from local list of buffers
local function checkbuffs()
    for buff, senders in pairs(buffers) do
        if not mq.TLO.Me.Buff(buff)() then
            -- get a random buffer that can cast the buff we want
            local candidates = {}
            for buffer, _ in pairs(senders) do
                table.insert(candidates, buffer)
            end

            -- once we have the random buffer, ask them to cast the buff
            local random_buffer = candidates[math.random(#candidates)]
            if random_buffer then
                --printf('Requesting %s from %s...', buff, random_buffer.character)
                actor:send(random_buffer, {id='beg', buff=buff}, function (status, message)
                    -- we have a reply here so that we can remove any buffers that didn't
                    -- clean up nicely (by calling /stopbuffbeg)
                    if status < 0 then removebuffer(random_buffer) end
                end)
            end
        end
    end
end

if mybuffs then bufferlogin() end
mq.delay(100)
beggarlogin()

-- we want to cleanup nicely so that all beggars know that we are done buffing
local runscript = true
mq.bind('/stopbuffbeg', function () runscript = false end)


-- ---------
-- Main Loop
-- ---------
while not terminate
do
    -- doevents() listens to ingame events
    -- Set each window title
    mq.cmdf("/SetWinTitle ${Me.Name}.${EverQuest.Server} (Lvl:${Me.Level} ${Me.Class} %s)", role)

    mq.doevents()
    if(role == "master" and uiinit==0) then
      checkbuffs()
      uiinit = 1
      mq.imgui.init('MainWindow', updateImGui)
    else
      -- Combat Routines
      if(Me.XTarget()>=1) then
        cleric_heals()
        berserker_attacks()
        magician_attacks()
      else
        checkbuffs()
        dobuffs()
      end
      if(Me.PctMana() <= 30 and not Me.Class() == "BER") then mq.cmd('/sit on') end
    end
    mq.delay(1) -- just yield the frame every loop
end
