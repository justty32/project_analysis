# GEMINI.md - Project Context: mylang (Scripting Language)

## Project Overview
`mylang` 是一個圖靈完備的 **Lisp 風格腳本語言**。它支援縮排語法、強大的引號擴展規則與閉包，旨在提供一個簡潔且強大的嵌入式執行環境。

### 核心特性 (Core Pillars)
- **二維 List 解析**：支援行、單詞與 **Tab 縮排** 解析。
- **引號擴展規則**：前向/後向嵌套速記法。
- **字串語法糖**：`"..."` 自動轉為 `(str "...")`，保留原始空白與換行。
- **元編程能力**：支援 Backquote (`` ` ``)、Unquote (`,`) 與 Unquote-Splicing (`,@`)。
- **動態執行環境**：基於 Dict 的變數儲存與 Lexical Scoping。
- **優先級順序**：`\` > 註解 > 縮排 > `"` > `'` > `(` > `{` > `[` > 單詞。

### 開發規範 (Strict Rules)
- **Parser 穩定性**：Parser 應產出純粹的 `list<symbol>`。核心語法糖（如字串包裝、容器轉譯）應保持簡潔。
- **邏輯實現於 Evaluator**：所有型別辨識、求值、標準函式庫皆在 `Evaluator` 處理。

## Implementation Plan (V3)

### Phase 1: Parser & Lexer Perfection (Completed)
- [x] **Lexer 精煉**：處理 `\` 優先級、多行字串、轉義字元、續行。
- [x] **Parser 語法糖**：引號擴展、Array/Dict/Set 轉換 (空格分隔)、字串包裝。
- [x] **Tab 縮排實作**：根據行首 Tab 數量判定 List 深度。

### Phase 2: Scripting Engine (Completed)
- [x] **環境管理**：實作支援閉包的 `Environment` (shared_ptr)。
- [x] **程序呼叫**：支援 NativeFunc 與 UserProcedure (支援多行 body)。
- [x] **核心系統函數**：實作 `get`, `exec`, `def`, `lambda`, `if`, `while`, `begin`。
- [x] **標準函式庫**：數學運算、比較運算 (含符號別名)、容器操作、檔案 IO。

### Phase 3: Runtime Expansion & Tooling (Current)
- [x] **REPL**：實作互動式命令行工具。
- [x] **Macro 系統強化**：實作 `backquote` / `unquote` / `unquote-splicing`。
- [ ] **錯誤處理**：提供運行時錯誤追蹤 (StackTrace) 與 `try-catch`。
- [ ] **效能優化**：考慮引入 Bytecode 與虛擬機 (VM)。
- [ ] **Binary IR**：支持 AST 序列化與加載。

## Useful Commands
- **Build:** `cmake -B build -G "MinGW Makefiles"`
- **Compile:** `cmake --build build`
- **Run Script:** `./build/mylang.exe <script_path>`
