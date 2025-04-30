# bushbot
While working with variious pair programming ai's here is a log of my recent introduction to this one ai in particular
deepseek
https://chat.deepseek.com/a/chat/s/7a46ac2f-e8a2-4602-9f70-9f43a81952b6
https://chat.deepseek.com/a/chat/s/437ba8d0-a408-4b15-94a1-3ae7a7ce5a26


https://chatgpt.com/c/67f9b273-1610-8002-a035-a0c122b81364

this link simplifies a lot of things:
https://chatgpt.com/c/6804550d-cbd4-8002-ba08-f4c025105356




# MacroQuest Plugin Developer Cheat Sheet

This reference guide covers essential MacroQuest C++ APIs, global pointers, macros, and TLO patterns for plugin development.

## 🧠 Character & Spawn Access

| What                         | Code / Macro                            | Description                                 |
|------------------------------|------------------------------------------|---------------------------------------------|
| Current character info       | `GetCharInfo()` → `CHARINFO*`            | Main player stats                           |
| Extended char info           | `GetCharInfo2()` → `CHARINFO2*`          | Spell bar, resistances, etc.                |
| Current target               | `pTarget` → `SPAWNINFO*`                 | Currently targeted spawn                    |
| Spawn by ID                  | `GetSpawnByID(spawnId)` → `SPAWNINFO*`   | Fetch any visible spawn                     |
| Group member                 | `GetGroupMember(i)` → `SPAWNINFO*`       | Group slot i (1-based index)                |

## 🔹 Spells

| What                         | Code                                     | Description                                 |
|------------------------------|------------------------------------------|---------------------------------------------|
| Spell struct by ID           | `GetSpellByID(spellId)` → `SPELL*`       | Access name, mana, target type, etc.        |
| Spell bar                    | `pChar2->MemorizedSpells[i]`             | ID of spell in gem `i`                      |
| Spell constants              | `NUM_SPELLS`, `NUM_SPELL_GEMS`           | Spell ID limits and bar size                |

## 🔹 Targeting & XTarget

| What                         | Code / Macro                            | Description                                 |
|------------------------------|------------------------------------------|---------------------------------------------|
| XTarget slots                | `pXTTargetMgr->XTargetSlots[i]`          | SpawnID per slot                            |
| Spawn from ID                | `GetSpawnByID(slot.SpawnID)`             | Get name, HP, etc.                          |

## 🔹 Commands & Interaction

| What                         | Code                                     | Description                                 |
|------------------------------|------------------------------------------|---------------------------------------------|
| Execute a command            | `DoCommand(pSpawn, "/sit")`              | As if typed into chat                       |
| Send chat output             | `WriteChatf("Text: %s", value)`          | Print to EQ window                          |
| Change window title          | `SetWindowText(hwnd, "EverQuest - Foo")` | Custom window title                         |
| Focus window                 | `SetForegroundWindow(hwnd)`              | Bring window to front                       |

## 🔹 Windows & MQ2 Utilities

| What                         | Code                                     | Description                                 |
|------------------------------|------------------------------------------|---------------------------------------------|
| EQ window handle             | `GetEQWindowHandle()` → `HWND`           | Main EverQuest window                       |
| Active window handle         | `GetForegroundWindow()`                  | Used to check active client                 |
| Is this EQ client active?    | `IsActiveClient()`                       | Custom helper for multi-box logic           |

## 🔹 TLO (Top-Level Object) via Code

| What                         | Code                                     | Description                                 |
|------------------------------|------------------------------------------|---------------------------------------------|
| Access any MQ TLO            | `ParseMacroData("${Me.Name}", buffer)`   | Run any TLO query                           |
| XTarget example              | `${Me.XTarget[1].Name}`                  | Use via `ParseMacroData`                    |

## 🔹 Plugin Lifecycle

| What                         | Code                                     | Description                                 |
|------------------------------|------------------------------------------|---------------------------------------------|
| Register a command           | `AddCommand("/foo", FooHandler)`         | Called during `InitializePlugin()`          |
| Unregister command           | `RemoveCommand("/foo")`                  | Called during `ShutdownPlugin()`            |
| Plugin init                  | `PLUGIN_API VOID InitializePlugin()`     | Entry point                                 |
| Plugin shutdown              | `PLUGIN_API VOID ShutdownPlugin()`       | Cleanup logic                               |

## 🧰 Common Headers

| Header               | What it’s for                            |
|----------------------|------------------------------------------|
| `MQ2Plugin.h`        | Core MQ plugin interface                 |
| `spdat.h`            | Spell constants + NUM_SPELLS             |
| `SpawnStruct.h`      | Structs for player/NPCs                 |
| `PlayerClient.h`     | CHARINFO, CHARINFO2, etc.                |
| `MQ2DataTypes.h`     | TLO definitions                          |
