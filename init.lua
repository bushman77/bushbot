local mq = require('mq')
local actors = require('actors')
local ImGui = require('ImGui')
local dannet = require('lib/dannet/helpers')
local Common = require('common')
local Magician = require('magician')
local Cleric = require('cleric')
local Berserker = require('berserker')
local Beastlord = require('beastlord')

-- imports
-- references
-- https://github.com/casssoft/imgui_lua_bindings
-- https://lemonate.io/docs/en/scripting/reference/imgui.html
-- capture global arguments and map them to table
local args = {...}
-- assign each element of global table to a human readable variable
local role = args[1]
local focus = "" 

-- Create specific NameSpaces for each Top Level Objects
local Cursor   = mq.TLO.Cursor
local DanNet   = mq.TLO.DanNet
local FindItem = mq.TLO.FindItem
local Group    = mq.TLO.Group
local Me       = mq.TLO.Me
local Pet      = mq.TLO.Pet
local Spawn    = mq.TLO.Spawn
local Spell    = mq.TLO.Spell
local Target   = mq.TLO.Target
local Zone     = mq.TLO.Zone

local terminate = false
local isOpen, shouldDraw = true, true
local uiinit = 0
local zoneshortname = ''

dannet.observe(Me.Name(), 'Zone.ID')

-- buffs each buffer type has
local mybuffs = {
    CLR={'Commitment', 'Shining Steel', 'Rallied Greater Aegis of Vie'},
    BST={"Spirit of Tala'Tak", 'Celerity', 'Shared Merciless Ferocity'},
    MAG={'Circle of Emberweave Coat', 'Volcanic Veil'}--, 'Radiant Modulating Shard'}
}
mybuffs = mybuffs[Me.Class.ShortName()]

-- ----------------
-- Public Functions
-- ----------------
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
local function group()
  for i=0,Group() do
    if(ImGui.Button(Group.Member(i).Name())) then
       focus = Group.Member(i) 
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
        if(ImGui.Button('Call Hero###callhero')) then
          mq.cmdf('/dex sandayar /targ %s', Target)
          mq.cmd('/dex sandayar /casting "Call of the Hero"')
        end
        if(ImGui.Button('SIT###sit')) then 
          mq.cmdf('dex %s /sit on', focus)
        end
      elseif(focus == 'Bushman') then
        if(ImGui.Button('SIT')) then mq.cmdf('/dex %s /sit on', focus) end
        --if(ImGui.Button('PACIFY')) then 
        --  mq.cmdf('/dex bushman /target %s', Target)
        --  mq.cmd('/dex bushman //casting ""')
        --  mq.cmdf('/dex %s /sit on', focus) 
        --end
      else
        if(ImGui.Button('SIT')) then mq.cmdf('/dex %s /sit on', focus) end
      end
    ImGui.EndTable()
    -- Always call ImGui.End if begin was called
  ImGui.End()  
end

-- actors
-- some example buffs, for demonstration purposes
-- due to BST crashing evertime Spirit of Tala'Tak is cast because it contains a special character in its name the int format is used instead 50228
local buff_queue = {}
local function dobuffs()
    for name, buff in pairs(buff_queue) do
      if(Me.Class()=="Cleric") then
        if buff=='Commitment' then Common.cast('Unified Hand of Infallibility', 10, name) end
        if buff=='Shining Steel' then Common.cast('Shining Steel Rk. II', 12, name) end
        if buff=='Rallied Greater Aegis of Vie' then
          Common.cast('Rallied Greater Aegis of Vie Rk. II',6, name) 
        end
      elseif(Me.Class()=="Beastlord") then
        if buff=="Spirit of Tala'Tak" then Common.cast(Spell(buff).ID(), 8, name) end
        if buff=='Celerity' then Common.cast(buff, 11,  name) end
        if buff=='Shared Merciless Ferocity' then Common.cast(buff, 6, name) end
      elseif(Me.Class()=="Magician") then
        if buff=='Circle of Emberweave Coat' then Common.cast(buff, 10, name) end
        if buff=='Volcanic Veil' then Common.cast(buff, 9, name) end
        if buff=='Group Perfected Levitation' then Common.cast_aa('Perfected Levitation', name) end
        if(not FindItem('Summoned: Radiant Modulation Shard')) then
          print("made it this far")
        end

      --if buff == 'Radiant Modulating Shard' then cast('Summon Modulating Shard', name) end
      end
      mq.delay(100)
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
        -- need to specify the actor here because we're sending to beggars
        -- from the buffer actor but leave it loose so that _all_ beggars
        -- receive this message
local function bufferlogin()
    for _, buff in ipairs(mybuffs) do actor:send({id='announce', buff=buff}) end
end

-- beggar login, request buffer buffs
local function beggarlogin() actor:send({id='buffs'}) end

-- beggar buff request, choose from local list of buffers
local function checkbuffs()
    for buff, senders in pairs(buffers) do
        if not Me.Buff(buff)() then
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

local function pet_buff(buff, spell)
  if Pet() then
    if Pet.Buff(buff)() then
    else
      mq.cmdf('/target %s', Pet())
      cast(spell, Pet())
    end
  else
    cast('Conscription Of Air', Me)
  end
end

local function mage_pet()
  item = "Folded Pack of Shak Dathor's Armaments"
  if(Me.Class()=='Magician') then
    if(Pet.ID()==0) then
      Common.cast('Conscription Of Earth',13 , Me)
    elseif(Me.Pet.Primary()==0) then
      if(Me.Inventory(30).Name.Find(item) ~=1) then 
        mq.cmdf('/target %s', Me.Name())
        cast(63811, 8, Me.Name())
        mq.delay(1000)
        if(Cursor.ID()) then mq.cmd('/autoinv') end
        mq.delay(500)
        mq.cmdf('/casting "%s"', item)
        mq.delay(1500)
        if(Cursor.ID()) then mq.cmd('/autoinv') end
        mq.delay(500)
        local bag = FindItem("Pouch of Quellious")

        if bag() then
          mq.cmdf('/itemnotify "%s" rightmouseup', bag.Name())
          local primary = FindItem("Summoned: Shadewrought Staff")
          if(primary()) then
            mq.cmdf('/itemnotify "%s" leftmouseup', primary.Name())
            mq.delay(500)
            mq.cmd('/pet give')
          end
          local secondary = FindItem("Sharewrought Mindmace")
          if(secondary()) then
            mq.cmdf('/itemnotify "%s" leftmouseup', secondary.Name())
             if(Cursor.ID()) then
               mq.cmd('/pet give')
             else
               print('No item on cursor to give!')
             end
          end
        else
         print("Bag not found in inventory.")
        end 
       while true do mq.delay(1) end 
      end
      -- after this spell si cast we will have a bag in our cursor
      -- this bag will be places in root inventory slot #30
    --else
     -- if(not Me.Aura('Arcane Distillect').ID()) then
     --   pet_buff('Arcane Distillect Effect', 'Arcane Distillect') 
     -- end
    --  pet_buff('Iceflame Barricade Rk.II', 'Iceflame Barricade Rk.II')
    --  pet_buff('Volcanic Veil', 'Volcanic Veil')
    --  pet_buff('Aegis of Rumblecrush', 'Aegis of Rumblecrush')
    --  pet_buff('Iceflame Barricade', 'Iceflame Barricade')
    end
    Magician.pet_buffs()
  end
end

-- we want to cleanup nicely so that all beggars know that we are done buffing
local runscript = true
mq.bind('/stopbuffbeg', function () runscript = false end)

-- ---------
-- Main Loop
-- ---------
Cleric.memorize_spells()
Magician.memorize_spells()
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
        Cleric.heals() 
        Berserker.attacks()
        Magician.attacks()
        Beastlord.attacks()
      else

        if(Me.PctMana() <= 60 and not Me.Class() == "BER") then
          print('Mana low')
          mq.cmd('/sit on') 
        end
        
        Magician.pet_buffs()
        checkbuffs()
        dobuffs()
      end
    end
    mq.delay(1) -- just yield the frame every loop
end
