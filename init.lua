local mq = require('mq')
local actors = require('actors')
local ImGui = require('ImGui')
local dannet = require('lib/dannet/helpers')
-- references
-- https://github.com/casssoft/imgui_lua_bindings
-- https://lemonate.io/docs/en/scripting/reference/imgui.html

-- capture global arguments and map them to table
local args = {...}
-- assign each element of global table to a human readable variable
local role = args[1]
local focus = {}
-- TLO mappedImGuiTableColumnFlags.WidthFixed, 10, 1
-- Create specific NameSpaces for each Top Level Objects
local Me     = mq.TLO.Me
local Target = mq.TLO.Target
local Spawn  = mq.TLO.Spawn

local terminate = false
local isOpen, shouldDraw = true, true
local uiinit = 0
local characters = {"Phrogeater", "Bushman", "Sandayar", "Solranacougar", "Zexerious", "Skullzsmasher"}

mq.cmd("/SetWinTitle ${Me.Name}.${EverQuest.Server} (Lvl:${Me.Level} ${Me.Class})")
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
  ImGui.Text("Hello")
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
      ImGui.Text(focus)
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
  for i in array do
    -- add 1 to the count varable
    count = count + 1
  end
  -- respond with the updated count value returned from for loop
  return count
end

-- ---------
-- Main Loop
-- ---------
while not terminate
do
    -- observer/3 sets up the observers
    --annet.observe(Me.Name(), Me.Level, 1000)
    --dannet.observe(Me.Name(), Target.ID, 1000)
    -- doevents() listens to ingame events
    mq.doevents()
    if(role == "master" and uiinit==0) then
      -- retrieves dannet peers
      --peers = mq.TLO.DanNet.Peers()
      -- iterate through the list of peers found in dannnet 
      --for i = 1, length(string.gmatch(peers, "(%w+)|")) do
        -- captures each peer in a local variable
        --local peer = mq.TLO.DanNet.Peers(i)()
        --print("Observations from " .. peer .. ":")
        -- inserts data into the characters table
        --table.insert(characters, peer)
        -- takes the each observed peer and splits the string on | delimiter and stores it in str variable
        --local str = string.gmatch(mq.TLO.DanNet(peer).Observe(), "(%w+)|")
        -- loops though the newly formed array from previous string and iterates though it
        --for j = 1, length(str) do
          -- isolates each observed peer and stores it in a varialbe
          --local observer = mq.TLO.DanNet(peer).Observe() 
          -- grabs the data from the observer if non is present default to "No Data" and stores the result in data variable
          --local data = mq.TLO.DanNet(peer).Observe() or "No Data"
          --print("[" .. observer .. "] -> " .. data)
        --end
      --end
      print("Should only appear once")
      uiinit = 1
      mq.imgui.init('MainWindow', updateImGui)
    else
      if(Me.XTarget()>=1 and Target.ID() and not Me.Combat()) then
        nexttarget("poo") 
      end
    end
    mq.delay(1) -- just yield the frame every loop
end
