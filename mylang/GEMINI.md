# GEMINI.md - Project Context: mylang (C++ Preprocessor)

## Project Overview
`mylang` 是一個圖靈完備的 **C++ 前置處理器與轉譯器**。它將 Lisp 風格的語法轉換為 C++ 原始碼，並在編譯階段進行元求值，具備處理複雜邏輯與動態生成代碼的能力。

### 核心特性 (Core Pillars)
- **C++ 代碼生成器**：元求值的字串結果直接輸出為 C++。
- **引號擴展規則**：強大的巢狀 List 速記法。
- **二維 List 解析**：按行與單詞解析原始碼。

## Implementation Plan (V3)

### Phase 1: Parser & Lexer Perfection (Current)
- [x] **Lexer 精煉**：正確處理 `\` 優先級、註解、續行（加空格）、省略引號的字串。
- [x] **Parser 語法糖**：實現引號擴展（對稱、前向、後向）、Array/Dict/Set 轉換。
- [ ] **移除舊遺產**：清理程式碼中遺留的點號 `.` 解析邏輯。

### Phase 2: Meta-Evaluator
- [ ] **實作元環境 (Meta-Environment)**：基於 Dict 的變數儲存。
- [ ] **C++ 輸出機制**：實作將 List 返回的字串 append 到產出檔案的邏輯。
- [ ] **Macro 與核心函數**：實作 Macro、`make`、`get`、`exec`、`def`。

### Phase 3: Binary IR & C++ Transpiler
- [ ] **Binary IR**：支持 AST 序列化。
- [ ] **高效映射**：優化 C++ 容器與型別的生成邏輯。

## Useful Commands
- **Build:** `cmake -B build -G "MinGW Makefiles"`
- **Compile:** `cmake --build build`
- **Run Parser Test:** `./build/mylang.exe`
