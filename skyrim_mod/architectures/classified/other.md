# Skyrim 雜項系統架構 (Other Systems)

本篇文檔記錄了那些不屬於 NPC 或物品核心系統，但對創造動態世界至關重要的次要引擎組件。

---

## 1. 天氣與環境 (Weather System)
天氣決定了視覺氛圍、光照與特定遊戲效果（如風暴傷害）。
- **類別**: `RE::TESWeather`
- **原始碼**: `include/RE/T/TESWeather.h`
- **C++ 操作**: 
    - 獲取當前天氣: `RE::Sky::GetSingleton()->currentWeather`
    - 強制切換天氣: `RE::Sky::GetSingleton()->ForceWeather(newWeather, true)`

---

## 2. 全球變數 (Global Variables)
全球變數是模組之間通訊與存儲狀態的簡易方式。
- **類別**: `RE::TESGlobal`
- **原始碼**: `include/RE/T/TESGlobal.h`
- **功能**: 通常用於存儲玩家的選擇（如：是否開啟某個模組功能），或是作為腳本中的條件判定（Conditions）。

---

## 3. 音效系統 (Audio System)
控制背景音樂與 3D 空間音效。
- **類別**: `RE::BSSoundHandle`
- **原始碼**: `include/RE/B/BSSoundHandle.h`
- **觸發音效**:
    ```cpp
    RE::BSSoundHandle handle;
    RE::BSAudioManager::GetSingleton()->BuildSound(handle, soundFormID);
    handle.Play();
    ```

---

## 4. 日曆與時間 (Calendar and Time)
追蹤遊戲內的天數、月份與精確小時。
- **類別**: `RE::Calendar`
- **原始碼**: `include/RE/C/Calendar.h`
- **功能**: 用於製作限時任務或週期性事件（如：每月第一天發放獎勵）。

---

## 5. 統計數據 (Tracked Stats)
追蹤玩家的犯罪記錄、殺敵數與探索進度。
- **類別**: `RE::PlayerCharacter` 內部的 `TESTrackedStatsEvent`
- **原始碼**: `include/RE/T/TESTrackedStatsEvent.h`

---

## 6. 深層 AI 行為修改 (Deep AI Modification)
修改 AI 的深層行為（而非僅僅是更換 Package）是可能的，但需要使用 C++ 進行底層函數攔截。
- **核心路徑**: `RE::AIProcess` 與 `RE::CombatController`。
- **技術手段**: 透過 Hook 引擎的 `Update` 或 `PickNextBehavior` 函數，可以直接接管 NPC 的決策邏輯。這允許實現如“集體戰術”、“動態逃跑算法”或“完全自定義的戰鬥風格”等高級功能。
- **詳見**: `architectures/commonlib-se/Deep_AI_Architecture.md`

## 7. 總結
這些系統雖然細碎，卻是構建複雜模組功能（如：根據天氣改變音效、根據時間切換全球變數）不可或缺的拼圖。
