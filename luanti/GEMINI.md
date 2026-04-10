# Luanti (formerly Minetest) 專案分析與開發指南 (GEMINI.md)

本文件定義了在使用 Gemini CLI 進行 Luanti 專案開發時的核心規範與流程，優先級高於系統預設指令。

## 核心指令與規範 (Mandates)

1.  **輸出語言：** 所有輸出、留檔、分析、教程及回應必須使用 **繁體中文 (Traditional Chinese)**。
2.  **自動留檔機制：** 在回覆任何技術細節、教學或分析時，必須同步將內容寫入 `work/` 下對應的子資料夾：
    -   架構分析：`work/architecture/`
    -   具體問題解答：`work/answer/`
    -   深度細節研究：`work/detail/`
    -   教學文件：`work/tutorial/`
3.  **程式碼標註規範：** 所有提到的程式碼片段必須標註其在專案中的原始碼位置（**完整檔案路徑與大約行號或函數名**）。
4.  **會話日誌管理：** 必須維持 `work/session_log.md`，詳盡紀錄每一次要求的具體執行事項（分析模組、解答問題、撰寫教程）。
5.  **會話進度保存：** 當收到「我準備要退出了」指令時，必須在 `work/gemini_temp/session_resume.md` 中彙整當前進度、分析路徑、待辦事項及上下文摘要。

## 專案概覽 (Project Overview)

Luanti 是一個使用 C++17 編寫的開源體素 (Voxel) 遊戲引擎，具備強大的 Lua Modding API。其核心架構圍繞著「引擎 (Engine)」、「遊戲 (Games)」與「模組 (Mods)」展開。

-   **核心技術棧：** C++17, Lua, CMake, Irrlicht MT (渲染), SDL2, SQLite3, LuaJIT.
-   **關鍵目錄：**
    -   `src/`: 引擎核心原始碼 (C++)。
    -   `builtin/`: 基礎 Lua API (`core` / `minetest` 命名空間)。
    -   `games/`: 遊戲定義 (如 `devtest`)，包含一組初始模組。
    -   `doc/`: 包含詳盡的 `lua_api.md` 與構建說明。

## 構建與運行 (Building and Running)

### Windows (使用 vcpkg)

```powershell
# 安裝依賴
vcpkg install zlib zstd curl[ssl] openal-soft libvorbis libogg libjpeg-turbo sqlite3 freetype luajit gmp jsoncpp gettext[tools] sdl2 --triplet x64-windows

# 配置與構建
cmake . -G"Visual Studio 16 2019" -DCMAKE_TOOLCHAIN_FILE=[VCPKG路徑]/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release -DRUN_IN_PLACE=TRUE
cmake --build . --config Release
```

### 運行指令

-   **客戶端：** `bin/luanti`
-   **專用伺服器：** `bin/luanti --server`
-   **單元測試 (C++)：** `bin/luanti --run-unittests`

## 開發規範 (Development Conventions)

-   **Lua API：** 優先使用 `core` 命名空間。
-   **Mod 開發：** 入口點為 `init.lua`，依賴定義於 `mod.conf`。
-   **分析路徑：** 應遵循 Level 1-6 的標準化路徑進行架構拆解。
-   **教學編寫：** 採目標導向，包含前置知識、原始碼導航、實作步驟與驗證方式。

## 工作空間結構 (Work Directory)

-   `work/architecture/`: 架構層次分析紀錄。
-   `work/tutorial/`: 功能開發教學。
-   `work/analysis/`: 通用邏輯分析。
-   `work/answer/`: 技術問題解答紀錄。
-   `work/detail/`: 特定模組深度研究。
-   `work/gemini_temp/`: 會話恢復與暫存資料。
