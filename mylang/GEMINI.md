# GEMINI.md - Project Context: mylang (C++ Preprocessor)

## Project Overview
`mylang` 是一個圖靈完備的 **C++ 前置處理器與轉譯器**。它將 Lisp 風格的語法轉換為 C++ 原始碼，並在編譯階段進行元求值。

### 核心特性 (Core Pillars)
- **C++ 代碼生成器**：元求值的字串結果直接輸出為 C++。
- **引號擴展規則**：強大的前向/後向嵌套速記法。
- **二維 List 解析**：支援行、單詞與 **Tab 縮排** 解析。
- **優先級順序**：`\` > 註解 > 縮排 > `"` > `'` > `(` > `{` > `[` > 單詞。
- **AST 規格**：Parser 輸出為純粹的 `list<symbol>`。

## Implementation Plan (V3)

### Phase 1: Parser & Lexer Perfection (Current)
- [x] **Lexer 精煉**：正確處理 `\` 優先級、註解、續行（加空格）、省略引號的字串。
- [x] **Parser 語法糖**：實現引號擴展（對稱、前向、後向）、Array/Dict/Set 轉換。
- [x] **Tab 縮排實作**：根據行首 Tab 數量判定 List 深度。
- [x] **AST 精簡化**：確保 Parser 輸出為純粹的 `list<symbol>`，移除過早的型別辨識（如數字）。

### Phase 2: Meta-Evaluator
- [x] **實作元環境 (Meta-Environment)**：基於 Dict 的變數儲存。
- [x] **C++ 輸出機制**：實作將 List 返回的字串 append 到產出檔案的邏輯。
- [x] **核心系統函數**：實作 `get` (不求值檢索)、`exec` (顯式求值)、`make` (構造)、`def` 系列。
- [ ] **Macro 系統**：實作自定義 Macro 展開邏輯。

### Phase 3: Binary IR & C++ Transpiler
- [ ] **Binary IR**：支持 AST 序列化。
- [ ] **高效映射**：優化 C++ 容器與型別的生成邏輯（如 `std::any` 應用）。

## Useful Commands
- **Build:** `cmake -B build -G "MinGW Makefiles"`
- **Compile:** `cmake --build build`
- **Run Parser Test:** `./build/mylang.exe`
