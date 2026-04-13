# GEMINI.md - Project Context: mylang (Scripting Language)

## Project Overview
`mylang` 是一個圖靈完備的 **Lisp 風格腳本語言**。它支援縮排語法、強大的引號擴展規則與閉包，旨在提供一個簡潔且強大的嵌入式執行環境。

### 核心特性 (Core Pillars)
- **二維 List 解析**：支援行、單詞與 **Tab 縮排** 解析。
- **引號擴展規則**：前向/後向嵌套速記法。
- **字串空白轉換**：字串中的連續空白自動轉為 `(ntimes_space X)` 以精確保留格式。
- **動態執行環境**：基於 Dict 的變數儲存與 Lexical Scoping。
- **優先級順序**：`\` > 註解 > 縮排 > `"` > `'` > `(` > `{` > `[` > 單詞。

### 開發規範 (Strict Rules)
- **Parser 穩定性**：Parser 應產出純粹的 `list<symbol>`。除非是為了實作核心語法糖（如字串空白處理），否則不應修改 Parser 與 Lexer。
- **邏輯實現於 Evaluator**：所有型別辨識、求值、標準函式庫皆在 `Evaluator` 處理。

## Implementation Plan (V3)

### Phase 1: Parser & Lexer Perfection (Completed)
- [x] **Lexer 精煉**：處理 `\` 優先級、多行字串、轉義字元、續行。
- [x] **Parser 語法糖**：引號擴展、Array/Dict/Set 轉換、字串空白轉換。
- [x] **Tab 縮排實作**：根據行首 Tab 數量判定 List 深度。

### Phase 2: Scripting Engine (Completed)
- [x] **環境管理**：實作支援閉包的 `Environment` (shared_ptr)。
- [x] **程序呼叫**：支援 NativeFunc 與 UserProcedure (支援多行 body)。
- [x] **核心系統函數**：實作 `get`, `exec`, `def`, `lambda`, `if`, `while`, `begin`。
- [x] **標準函式庫**：數學運算 (`+`, `-`, `*`, `/`, `mod`)、比較 (`>`, `==`, `!=` 等)、容器操作 (`push`, `at`, `len`, `has?`)。
- [x] **檔案 IO**：實作 `io_read`, `io_write`, `io_exists?`。

### Phase 3: Runtime Expansion & Tooling (Current)
- [x] **REPL**：實作互動式命令行工具 (支援多行括號偵測)。
- [ ] **Macro 系統展開**：完善 `backquote` / `unquote` 語法以強化元編程。
- [ ] **Binary IR**：支持 AST 序列化與加載。
- [ ] **錯誤處理**：提供運行時錯誤追蹤 (StackTrace)。

## Useful Commands
- **Build:** `cmake -B build -G "MinGW Makefiles"`
- **Compile:** `cmake --build build`
- **Run Script:** `./build/mylang.exe <script_path>`
