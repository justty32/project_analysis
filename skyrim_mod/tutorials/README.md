# Skyrim C++ 插件開發系列教學

本教學將引導您使用 `CommonLibSSE-NG` 從零開始開發一個高性能的 C++ 插件 (SKSE Plugin)。

## 教學目錄

1. [階段一：環境準備與項目建立](01_Environment_Setup.md) - 安裝工具、配置 Vcpkg 並建立第一個項目。
2. [階段二：代碼開發實戰](02_Coding_Basics.md) - 解析入口函數、註冊事件並實現一個功能（例如：按下快捷鍵顯示玩家位置）。
3. [階段三：構建與部署](03_Build_and_Deploy.md) - 使用 CMake 構建 DLL、將插件安裝到遊戲中並進行測試。

## 進階實戰系列 (龍吼功能實現)

4. [進階：造物之術 (生成床鋪)](04_Shout_Spawn_Bed.md) - 學習如何在準星指向的位置動態生成對象。
5. [進階：點物成金 (轉化起司)](05_Shout_Transmute_Barrel.md) - 學習如何識別並替換遊戲世界中的現有物品。
6. [進階：備戰之音 (背包檢查)](06_Shout_Inventory_Check.md) - 學習如何遍歷玩家物品欄並根據條件給予獎勵。
7. [進階：瞬時營地 (生成房屋)](07_Shout_Spawn_House.md) - 學習如何生成大型靜態建築並處理坐標偏移。
8. [進階：奧法之石 (啟動贈禮與技能學習)](08_Startup_Item_Learn_Spell.md) - 學習如何自動給予物品並實現點擊物品觸發邏輯。
9. [進階：馴服之聲 (修改 NPC 行為)](09_Modifying_NPC_Behavior.md) - 學習如何修改 NPC 屬性、停止戰鬥並強制播放動畫。
10. [進階：言不由衷 (干預對話系統)](10_Modifying_NPC_Dialogue.md) - 學習如何監聽對話事件、獲取說話者並動態操作字幕。
11. [進階：地利之便 (偵測城鎮位置)](11_Locating_Player_Town.md) - 學習如何透過地理坐標與區域系統判斷玩家身處何地。
12. [進階：移山倒海 (生成自然景觀)](12_Terrain_Modification.md) - 學習如何在準星位置生成山石樹木，並了解地形修改的技術邊界。

## 高級實戰：解決技術難點

13. [高級：存檔安全 (持久化與回收)](13_Advanced_Spawning_Persistence.md) - 解決生成對象重啟消失與存檔膨脹問題。
14. [高級：安全轉化 (物品轉移)](14_Advanced_Inventory_Transfer.md) - 解決容器內容物在轉化過程中遺失的問題。
15. [高級：靈魂附魔 (操作 ExtraData)](15_Advanced_Item_ExtraData.md) - 深入學習如何動態修改物品的名稱、強化等級與附魔。
16. [高級：導航剪裁 (Navmesh Cutting)](16_Advanced_Navmesh_Cutting.md) - 探討如何讓 NPC 識別並繞過動態生成的建築。
17. [高級：深層交互 (UI Hooking)](17_Advanced_UI_Hooking_Inventory.md) - 學習如何 Hook 物品欄選單，讓任何物品都變得可交互。

## NPC 與 實體 操作進階

18. [進階：召喚之術 (生成 NPC)](18_Generating_NPC_From_Template.md) - 學習如何透過模板在世界中動態生成 NPC 並設置其行為。
19. [高級：基因改造 (修改模板)](19_Advanced_Modifying_NPC_Templates.md) - 學習如何直接干預 NPC 模板數據，實現大規模的屬性調整。
20. [高級：無中生有 (動態創建新物品)](20_Dynamic_Form_Creation.md) - 學習如何使用 IFormFactory 在不依賴 ESP 的情況下，動態鍛造全新的專屬武器。
22. [高級：靈魂主宰 (深層 AI Hooking)](22_Deep_AI_Modification.md) - 學習如何 Hook AI 更新循環，實現對 NPC 決策邏輯的底層接管。

## Radiant AI 實戰與擴充

23. [進階：生活掌控者 (強制沙盒)](23_Expanding_Sandboxing.md) - 學習如何強迫 NPC 使用特定的家具並執行沙盒動作。
24. [進階：命運編織者 (故事管理器介入)](24_Story_Manager_Intervention.md) - 學習如何手動觸發事件節點來啟動動態任務。
25. [進階：任務竊取者 (別名操控)](25_Quest_Alias_Manipulation.md) - 學習如何在任務運行中動態更換任務目標。
26. [進階：紅蓮之劍 (動態附魔與特效)](26_Apply_Enchantment_and_Visuals.md) - 學習如何透過龍吼觸發，為手中武器添加火焰視覺特效與實質附魔屬性。
27. [進階：起司之觸 (材質覆蓋與模型掛載)](27_Cheese_Touch_Texture_and_Mesh.md) - 學習如何動態覆蓋物體貼圖，並在模型樹中插入自定義的 3D 裝飾品。
28. [進階：倍化之術 (持續性體型增長)](28_Continuous_Growth_Spell.md) - 學習如何實現引導類法術，動態修改並同步更新生物的 3D 縮放比例。
29. [進階：起司吊墜 (動態物理掛載)](29_Advanced_Physics_Attachments.md) - 學習如何在模型上建立 Havok 物理約束，實現隨動作擺動的掛飾效果。
30. [進階：流星火雨 (地面魔法陣與投射物)](30_Meteor_Fire_Rain_Spell.md) - 學習如何使用貼花系統製作地形魔法陣，並實現從天而降的密集投射物攻擊。
31. [終極複合：巡航馬車 (動態載具與掛載)](31_Advanced_Moving_Carriages.md) - 學習如何利用 TranslateTo 平滑移動實體，並同步掛載 NPC 與玩家座標實現動態載具。
32. [進階：萬物皆可飛 (物體巡航飛行)](32_Object_Cruise_Flight.md) - 學習如何讓世界中的任何物品脫離重力，沿著預設軌跡巡航飛行。

## 終極挑戰：純代碼創世之舉 (動態城鎮)

這是一個極具挑戰性的專題，探討如何完全透過 C++ 插件建立一座城市。

- [01. 概念與極限](Ultimate_Town_Challenge/01_The_Concept_and_Limits.md) - 探討純代碼生成城鎮的步驟，以及 Navmesh 與 LOD 帶來的技術極限。
- [02. 動態傳送門 (讓房屋可進入)](Ultimate_Town_Challenge/02_Dynamic_Interiors_and_Doors.md) - 深入解析 `ExtraTeleport` 機制，教導如何動態鏈接外部世界與室內單元。
- [02.5. 模塊製作與定制](Ultimate_Town_Challenge/02.5_Module_Creation_and_Customization.md) - 教導如何設計適合拼接的模組，以及如何透過 C++ 動態更換貼圖與裝飾。
- [03. 模塊化拼接 (Tile-set Snapping)](Ultimate_Town_Challenge/03_Modular_Snapping_System.md) - 學習如何利用接點數據，將帶有 Navmesh 的模組精確對齊。
- [04. 程序化室內生成](Ultimate_Town_Challenge/04_Modular_Interiors.md) - 探索如何利用模塊在空白單元中拼湊出無限變化的室內格局。
- [05. 動態導航與尋路 (NavMesh Cutting)](Ultimate_Town_Challenge/05_Advanced_Dynamic_Navigation.md) - 學習利用 `NavMeshObstacleManager` 讓 NPC 學會避開你生成的建築物。
- [06. 經濟與人口管理 (Economy & Population)](Ultimate_Town_Challenge/06_Economy_and_Population.md) - 展示如何動態分配鐵匠等商人，並建立運作中的商箱系統。
- [07. 城市防禦系統 (City Defense)](Ultimate_Town_Challenge/07_City_Defense_Systems.md) - 教導如何監聽戰鬥事件，在巨龍來襲時動態生成弩車等防禦工事。
- [08. 自動化地圖標記 (Map Markers)](Ultimate_Town_Challenge/08_Automated_Map_Markers.md) - 學習操作 `ExtraMapMarker`，讓玩家能在大地圖上快速旅行回動態城鎮。

## 為什麼選擇 CommonLibSSE-NG？
- **跨版本支持**: 同一套代碼支持 SE、AE 和 VR 版本。
- **現代 C++**: 使用 C++23 標準，開發體驗更佳。
- **豐富的 API**: 涵蓋了幾乎所有的遊戲引擎內部接口。

> **準備好了嗎？** 請先閱讀 [階段一：環境準備與項目建立](01_Environment_Setup.md)。
