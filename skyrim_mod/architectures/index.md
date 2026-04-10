# Skyrim 模組開發架構索引 (Modding Architecture Index)

歡迎來到 Skyrim 模組開發技術文檔庫。本索引旨在協助開發者快速導航至特定的技術領域、引擎底層機制或實戰教學。

---

## 📂 核心庫指南：CommonLibSSE-NG
提供了 `CommonLibSSE-NG` 的基礎知識與開發起步指南。

*   **[概覽與學習路徑](commonlibsee-ng/README.md)** - 必讀！包含了整個開發環境的推薦學習順序。
*   **[基礎架構 (Architecture)](commonlibsee-ng/00_overview.md)** - 了解 CommonLibSSE-NG 的核心目標與技術特性。
*   **[目錄結構 (Structure)](commonlibsee-ng/01_structure.md)** - 快速了解專案佈局與關鍵文件位置。
*   **[逆向工程層 (RE Layer)](commonlibsee-ng/RE/00_overview.md)** - 深入探討遊戲引擎內部的 C++ 類別。
    *   關鍵類別：`TESForm` (數據根基), `Actor` (角色), `TESObjectREFR` (世界引用)。
*   **[開發示例 (Examples)](commonlibsee-ng/examples/00_helloworld.md)** - 從 Hello World 到 Hooking 技術的實作範例。

---

## 📂 主題分類技術文檔 (Classified Deep Dives)
針對遊戲不同系統的深度解析，每個子目錄通常包含：
- **Architecture**: 引擎內部邏輯與理論背景。
- **Answers**: 針對特定問題的實作方案。
- **Tutorials**: 帶有代碼片段的步驟導引。

### 🎨 [3D 圖形與渲染 (3D Graphics)](classified/3D_Graphics/)
*   **核心**: NIF 檔案格式、Scene Graph (NiNode)、材質著色器、特效系統。
*   **重點文檔**: [NIF 格式深度解析](classified/3D_Graphics/Architecture/NIF_File_Format_Deep_Dive.md)、[VFX 架構](classified/3D_Graphics/Architecture/VFX_Architecture.md)。

### 👥 [NPC 與 AI 行為 (NPC)](classified/NPC/)
*   **核心**: Actor 解剖學、Radiant AI、Havok Behavior (動畫狀態機)、骨骼與蒙皮。
*   **重點文檔**: [Actor 動作架構](classified/NPC/Architecture/Actor_Animation_Architecture.md)、[Havok Behavior 深度解析](classified/NPC/Architecture/Havok_Behavior_Deep_Dive.md)。

### 🪄 [魔法與特效 (Magic)](classified/Magic/)
*   **核心**: 法術定義 (SPEL/MGEF)、投射物物理、魔法效果流轉、天賦系統 (Perk)。
*   **重點文檔**: [法術系統架構](classified/Magic/Architecture/Spell_System_Anatomy.md)、[投射物碰撞機制](classified/Magic/Answers/Projectile_Impact_and_Effect_Flow.md)。

### ⚔️ [道具與物品欄 (Items)](classified/Items/)
*   **核心**: 物品原型與實體、容器架構、工匠系統、UI 鉤子。
*   **重點文檔**: [物品定義與存儲](classified/Items/Answers/Item_Definition_and_Storage.md)、[容器架構解析](classified/Items/Architecture/Container_Architecture.md)。

### 🗣️ [對話與任務 (Dialogue & Quest)](classified/Dialogue_Quest/)
*   **核心**: 任務生命週期、故事管理器 (Story Manager)、對話話題機制。
*   **重點文檔**: [任務與對話存儲架構](classified/Dialogue_Quest/Architecture/Dialogue_Quest_Storage_Architecture.md)、[對話話題運作機制](classified/Dialogue_Quest/Architecture/Dialogue_Topic_Mechanism.md)。

### 🌍 [世界、環境與空間 (World)](classified/World/)
*   **核心**: 室內/室外加載機制、世界交互觸發、自定義世界空間、載具系統。
*   **重點文檔**: [室內 vs 室外架構](classified/World/Answers/Exteriors_vs_Interiors.md)、[世界運行機制](classified/World/Architecture/World_Runtime.md)。

### ⚙️ [系統底層與工具 (Systems)](classified/Systems/)
*   **核心**: ESP/ESM 結構、BSA 資源封裝、Papyrus 腳本綁定、碰撞系統。
*   **重點文檔**: [TESForm 詳細解析](classified/Systems/Architecture/TESForm_Detailed.md)、[Papyrus 綁定架構](classified/Systems/Architecture/Papyrus_Binding_Architecture.md)。

### 💾 [存檔與序列化 (Save Data)](classified/Save_Data/)
*   **核心**: ESS 檔案結構、ChangeFlags、SKSE 數據持久化。
*   **重點文檔**: [ESS 存檔深度解析](classified/Save_Data/Architecture/ESS_Save_Deep_Dive.md)。

### 🌀 [雜項系統 (Others)](classified/other.md)
*   涵蓋天氣系統、全球變數、音效、日曆時間等次要組件。

---

## 💡 如何使用本手冊
1.  **初學者**: 請從 `commonlibsee-ng/README.md` 開始。
2.  **查找特定功能**: 根據上方的主題分類進入對應子目錄。
3.  **解決 Bug**: 優先閱讀 **Architecture** 部分以理解引擎原始設計，再參考 **Answers** 中的常見坑點。
