# mylang 架構設計文件 (Architecture)

## 1. 系統流程 (System Flow)

1. **Lexer (詞法分析)**：
   - 處理最高優先級的 `\` (轉義與續行)。
   - 過濾註解 `//`, `/**/`。
   - 解析字串 `"` 與引號 `'`。**字串支援多行內容與內部反斜線續行**。
   - **層級判定**：根據括號深度與行首 **Tab** 數量判定邏輯行與 List 深度。
2. **Parser (語法解析)**：
   - 將 Token 流轉換為純粹的 `list<symbol>` (AST)。
   - **語法糖轉譯**：處理引號擴展與容器（Array/Dict/Set）的初步轉換。
3. **Evaluator (求值執行)**：
   - 執行 Lisp 邏輯。
   - **動態型別**：在求值時根據內容將符號轉為數字或布林值。
   - **作用域管理**：使用基於 `shared_ptr` 的 `Environment` 樹，支援閉包與局部變數。
   - **程序呼叫**：支援原生 C++ 函數（NativeFunc）與用戶自定義程序（UserProcedure）。

## 2. 資料結構 (Data Model)
- **LIST**: `std::list<Value>`。
- **ATOM**: `std::string` (AST 基礎)。
- **NUMBER**: `double` (求值結果)。
- **PROCEDURE**: 包含參數、主體與閉包環境的結構。
- **CONTAINERS**: 由語法糖產生的 `vector`, `map`, `set`。

## 3. 核心設計原則
- **通用腳本導向**：提供完整的程式語言功能。
- **延遲決策 (Lazy Decision)**：解析階段保持 AST 的原始性（純粹的 `list<symbol>`），執行階段（Evaluator）才根據上下文決定 Symbol 的語義與型別。
- **Parser 穩定性**：一旦 Parser 產出穩定的 AST 結構，應避免修改解析邏輯，所有的功能擴充應透過擴展 `Evaluator` 的標準函式庫或 `Macro` 系統來達成。
- **簡潔語法**：透過 Tab 縮排減少括號的使用負擔。
- **高擴展性**：核心僅提供基礎 Macro，進階功能透過 Macro 系統自行定義。
