# GEMINI.md - Project Context: mylang (Scripting Language)

## Project Overview
`mylang` 是一個圖靈完備的 **Lisp 風格腳本語言**。它支援縮排語法、強大的引號擴展規則與閉包，旨在提供一個簡潔且強大的嵌入式執行環境。

### 核心特性 (Core Pillars)
- **二維 List 解析**：支援行、單詞與 **Tab 縮排** 解析。
- **引號擴展規則**：前向/後向嵌套速記法。
- **動態執行環境**：基於 Dict 的變數儲存與 Lexical Scoping。
- **優先級順序**：`\` > 註解 > 縮排 > `"` > `'` > `(` > `{` > `[` > 單詞。

### 開發規範 (Strict Rules)
- **不可修改 Parser 與 Lexer**：語法解析層級必須保持穩定。Parser 應產出純粹的 `list<symbol>` (AST)。
- **邏輯實現於 Evaluator**：所有型別辨識、求值、語法糖轉譯與標準函式庫，皆必須在 `Evaluator` 或 `Macro` 階段處理。
- **基於 `list<symbol>` 操作**：一切求值操作皆必須建立在 Parser 產出的 `list<symbol>` 結構之上。

## Implementation Plan (V3)

### Phase 1: Parser & Lexer Perfection (Completed)
- [x] **Lexer 精煉**：正確處理 `\` 優先級、註解、續行（加空格）、省略引號的字串。
- [x] **Parser 語法糖**：實現引號擴展（對稱、前向、後向）、Array/Dict/Set 轉換。
- [x] **Tab 縮排實作**：根據行首 Tab 數量判定 List 深度。
- [x] **AST 精簡化**：確保 Parser 輸出為純粹的 `list<symbol>`。

### Phase 2: Scripting Engine (Current)
- [x] **環境管理**：實作支援閉包的 `Environment` (shared_ptr)。
- [x] **程序呼叫**：支援 NativeFunc 與 UserProcedure。
- [x] **核心系統函數**：實作 `get`, `exec`, `def`, `lambda`, `if`。
- [ ] **標準函式庫**：擴展數學運算、邏輯運算與容器操作函數。
- [ ] **Macro 系統展開**：完善用戶自定義 Macro 的展開邏輯。

### Phase 3: Runtime Expansion & Tooling
- [ ] **REPL**：實作互動式命令行工具。
- [ ] **Binary IR**：支持 AST 序列化與加載。
- [ ] **錯誤處理**：提供更詳細的運行時錯誤追蹤（StackTrace）。

## Useful Commands
- **Build:** `cmake -B build -G "MinGW Makefiles"`
- **Compile:** `cmake --build build`
- **Run Script:** `./build/mylang.exe <script_path>`
