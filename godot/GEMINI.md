# Godot Engine - GEMINI 上下文與開發規範

歡迎來到 Godot Engine 原始碼庫。Godot 是一款功能齊全、跨平台的 2D 與 3D 遊戲引擎。

## 專案概覽 (Project Overview)

-   **核心技術：** C++ (引擎核心), Python (SCons 構建系統), GLSL (著色器)。
-   **版本：** 4.7.dev (Godot 4.x 系列)。
-   **架構分層：**
    -   `core/`: 基礎類型、數學、記憶體管理與低階引擎邏輯。
    -   `scene/`: 場景樹系統、節點與資源管理。
    -   `servers/`: 渲染、物理、音訊等底層抽象層。
    -   `editor/`: Godot 編輯器實作。
    -   `modules/`: 可選模組與外掛功能 (如 GDScript, 物理引擎, 特定檔案格式)。
    -   `platform/`: 平台相關實作 (Windows, Linux/BSD, macOS, Android, iOS, Web)。
    -   `drivers/`: 底層硬體驅動 (Vulkan, OpenGL, D3D12 等)。
    -   `thirdparty/`: 專案隨附的第三方函式庫。

## 構建與執行 (Building and Running)

Godot 使用 **SCons** 作為構建系統。需要 Python 3.9+ 以及 SCons 4.0+。

### 常用構建指令

-   **構建編輯器 (預設)：**
    ```bash
    scons platform=<platform> target=editor -j<number_of_cores>
    ```
    *(例如：`scons platform=windows target=editor -j8`)*

-   **構建匯出範本 (Export Templates)：**
    ```bash
    scons platform=<platform> target=template_release -j8
    scons platform=<platform> target=template_debug -j8
    ```

-   **清理構建：**
    ```bash
    scons platform=<platform> --clean
    ```

### 平台名稱參數：
`windows`, `linuxbsd`, `macos`, `android`, `ios`, `web`。

## 開發慣例與規範 (Development Conventions)

-   **代碼風格：** 遵循 Godot 專屬 C++ 風格。強制使用 Clang-format (`.clang-format` 位於根目錄)。
-   **C++ 標準：** Godot 4 使用 C++17。
-   **Include 路徑：** 使用根目錄相對路徑 (例如：`#include "core/object/object.h"`)。
-   **單例模式 (Singletons)：** 多數核心系統透過單例訪問 (例如：`RenderingServer::get_singleton()`)。
-   **記憶體管理：** 使用 `memnew()`, `memdelete()` 以及帶參照計數的物件 (`Ref<T>`)。禁止使用標準 `new`/`delete`。
-   **錯誤處理：** 使用巨集如 `ERR_FAIL_COND()`, `ERR_FAIL_NULL()` 與 `DEV_ASSERT()`。

---

## 專案分析與操作標準作業流程 (SOP)

根據 `work/project_analysis_workflow.md`，AI 助手必須嚴格遵循以下規範：

### 1. 語言與文件輸出
-   **強制語言：** 所有輸出、對話與留檔必須使用 **繁體中文**。
-   **自動留檔：** 任何技術細節、教學或分析內容，必須同步寫入 `work/` 下對應的子資料夾。

### 2. 目錄與日誌規範
-   **空間結構：**
    -   `work/architecture/`: 架構分析文件。
    -   `work/tutorial/`: 開發教學。
    -   `work/analysis/`: 專題分析。
    -   `work/answer/`: 問題解答記錄。
    -   `work/detail/`: 細部實作紀錄。
    -   `work/gemini_temp/`: 臨時或進度保存檔案。
-   **會話日誌：** 維持 `work/session_log.md`，詳盡記錄每一次執行的事項 (分析模組、解答問題、撰寫教學)。
-   **程式碼標註：** 所有片段必須標註 **檔案路徑與大約行號或函數名**。

### 3. 進度保存 (Session Checkpoint)
-   當使用者提出「我準備要退出了」時，必須在 `work/gemini_temp/session_resume.md` 彙整當前的專案理解、完成的分析路徑、剩餘待辦事項與上下文摘要。

### 4. 分析路徑 (Architecture Analysis Path)
分析應依序執行以下層級並在 `work/architecture/` 留檔：
- **Level 1**: 初始探索與基礎架構 (README, Workspace, 依賴)。
- **Level 2**: 核心模組職責 (Entry points, 權責劃分)。
- **Level 3**: 進階機制 (AI, 生成, 模擬)。
- **Level 4**: 遊戲性系統 (戰鬥, 技能, 背包)。
- **Level 5**: 技術架構與數據流 (網路, 協定, 存檔格式)。
- **Level 6**: 視覺與渲染 (Shaders, Mesh, 骨架, 特效)。

### 5. 教學編寫規範 (Tutorial Framework)
教學檔案存放於 `work/tutorial/`，結構包含：
1. **目標導向**：解決具體開發問題。
2. **前置知識**：關聯核心模組。
3. **原始碼導航**：標註具體修改路徑與行號。
4. **實作步驟**：代碼實作或配置修改。
5. **驗證方式**：測試與驗證。
