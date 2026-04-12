# 進度存檔 — TE4 架構文件與教學系列

> 下次對話開始時，請先讀這份文件，再繼續工作。
> 最後更新：2026-04-11

---

## 已完成工作

### architecture/ 架構文件（全部以繁體中文撰寫）

| 檔案 | 內容 |
|------|------|
| `architecture/overview.md` | 整體架構（C 層 + Lua 層 + 模組層） |
| `architecture/engine_detail.md` | C 子系統（SDL2 / OpenGL / PhysFS / src/ 下所有 .c） |
| `architecture/lua_engine_detail.md` | 引擎 Lua 詳細（OOP / Entity / Game Loop / Resolver / UI） |
| `architecture/module_dev_guide.md` | 模組開發參考指南 |
| `architecture/game_detail.md` | game/ 目錄：loader、ToME 核心類別（Game/Actor/Player/NPC/Party/Zone）、介面（Combat/AI/Archery）、AI scripts、birth/race/class、zones、addons |
| `architecture/tome_content.md` | ToME 內容層深度分析：data/general/（NPC/物品/地形/商店/陷阱/事件）、mod/dialogs/（44+ 對話視窗）、data/quests/（49 任務）、data/lore/（36 史料）、mod/class/generator/（自訂產生器）、mod/class/uiset/（Classic/Minimalist UI） |

### architecture/tutorials/ 教學系列

| 檔案 | 狀態 | 內容摘要 |
|------|------|----------|
| `00-overview.md` | ✅ 完成 | 教學總覽、三條學習路線、TODO 清單 |
| `01-hello-dungeon.md` | ✅ 完成 | 最小可執行模組（1663 行），15 步驟，含所有必要檔案完整程式碼 |
| `05-advanced-ai.md` | ✅ 完成 | 進階 AI 系統：newAI/runAI、ai_state、tactical 表、improved_tactical 三步評分、ai_tactic 人格範本、4 級 NPC 範例、自訂戰術擴充 |
| `06-addon-dev.md` | ✅ 完成 | Addon 開發：init.lua、hooks/superload/overload 三機制、完整「暗影刺客職業」範例、Superload 鏈原理、打包發布 |

---

## 待完成教學（00-overview.md 中標記為 TODO）

下次對話請**依序**完成以下教學：

### 教學 02：加入物品系統
**範圍**：揹包、裝備欄、消耗品、物品掉落

關鍵系統：
- `engine/interface/ActorInventory.lua` — 揹包、裝備欄 API
- `engine/Object.lua` — 物品 Entity 基礎類別
- `resolvers.inventory`、`resolvers.equip`、`resolvers.drops`
- `zone.lua` 的 `object` generator 設定
- `engine/generator/object/Random.lua`
- ToME 物品 ego 系統（`data/general/objects/`）

建議檔案結構（延伸教學 01 的 hellodungeon 模組）：
```
data/general/objects/weapons.lua    ← 定義可撿取的武器
data/general/objects/potions.lua    ← 消耗品
class/Object.lua                    ← 繼承 engine.Object
load.lua                            ← 加入 Object 類別初始化
```

---

### 教學 03：加入多個地區與世界地圖
**範圍**：Zone 切換、WorldNPC、Wilderness（世界地圖）

關鍵系統：
- `engine/Zone.lua` — Zone 切換 API（`game:changeLevel`）
- `engine/Level.lua` — Level 物件
- `game/modules/tome-1.7.6/mod/class/WorldMap.lua` — 世界地圖實作
- `data/zones/` 目錄結構
- `zone.lua` 的 `on_enter`、`on_leave` 回調
- 傳送點（transition tiles）設定

建議範例：從地城入口進入地城，完成後回到城鎮（2 個 Zone）

---

### 教學 04：加入任務系統
**範圍**：Quest 定義、任務狀態追蹤、NPC 對話

關鍵系統：
- `engine/Quest.lua` — Quest 基礎類別
- `engine/Chat.lua` — 對話腳本（.lua chat 格式）
- `data/quests/` 目錄（ToME 有 49 個任務可參考）
- `game.player:grantQuest()`、`game.player:hasQuest()`
- NPC 的 `on_die` 觸發任務完成

---

### 教學 07：為 ToME 新增職業與技能樹（進階）
**範圍**：延伸教學 06，深入 Birther 系統、技能樹 UI、精通系統

這是教學 06 的進階版，聚焦在：
- `newTalentType` 的所有選項（`generic`、`min_require`、`not_on_random_boss`）
- `resolvers.talents`、`resolvers.equipbirth`、`resolvers.inventorybirth`
- Birth 描述符的 `descriptor_choices`（種族/職業相容性限制）
- 多個天賦的連鎖依賴（`require.talent`）
- 讓 Addon 職業與現有種族完整整合

---

## 重要技術記憶點

### 路徑規則
- 引擎 Lua：`game/engines/engine/` → `require "engine.X"`
- ToME 模組 Lua：`game/modules/tome-1.7.6/mod/` → `require "mod.X"`
- Addon data：`game/addons/<name>/data/` → `/data-<name>/`（PhysFS 掛載後）
- 引擎 AI：`game/engines/engine/ai/` → `require "engine.ai.X"`（不是 `te4-1.7.6/` 子目錄）

### 教學風格要求
- 全繁體中文
- 每步驟都要有完整可執行的程式碼片段
- 解釋「為什麼」，不只是「怎麼做」
- 常見錯誤排查表

### 已確認的 init.lua 欄位
```lua
hooks = true / superload = true / overload = true / data = true
cheat_only = true   -- 僅 cheat 模式（tome-addon-dev）
dlc = 5             -- DLC 驗證（tome-possessors）
weight = 1          -- 載入優先順序
```

### Superload 鏈的正確範本
```lua
local _M = loadPrevious(...)   -- 必須傳 ...
local orig_method = _M.method
function _M:method(...)
    -- 自訂邏輯
    return orig_method(self, ...)
end
return _M                      -- 必須 return
```

### 重要 Hook 名稱
- `ToME:run` — 遊戲啟動（主選單前）
- `ToME:load` — mod/load.lua 末尾（最適合載入天賦/職業定義）
- `ToME:birthDone` — 角色創建完成
- `Entity:loadList` — 每次 NPC/物品列表載入時
- `MapGeneratorStatic:subgenRegister` — 靜態地圖子產生器
- `Game:changeLevel` — 切換樓層
- `Actor:move`、`Actor:onWear`、`Actor:preUseTalent`

---

## 快速確認現有檔案

```bash
ls architecture/tutorials/
# 應該看到：00-overview.md, 01-hello-dungeon.md, 05-advanced-ai.md, 06-addon-dev.md

ls architecture/
# 應該看到：overview.md, engine_detail.md, lua_engine_detail.md,
#            module_dev_guide.md, game_detail.md, tome_content.md, tutorials/
```
