# AI Agent Server 進階擴充 (Extra)

當核心隊列流水線穩定後，可考慮以下更具前瞻性的功能：

## 1. 人機協作鎖定 (Human-in-the-loop Hook)
*   **構想**：流水線中增加一個 `HumanReviewModule`。
*   **場景**：當 Agent 決定進行高風險操作（如刪除大量檔案）時，`tags["status"]` 會更新為 `awaiting_user_approval`。
*   **實作**：伺服器暫停該 Task 的流轉，並向前端（Client）發送確認請求，直到使用者回傳 API 為止。
*   **價值**：在高風險任務中兼顧安全性與自動化。

## 2. 工作空間自動索引 (Workspace Indexing & RAG)
*   **構想**：背景維護一個對 `workspace/` 內檔案的向量索引。
*   **場景**：每當 Agent 執行 `CREATE` 或 `MODIFY` 後，流水線尾端投遞給一個 `IndexerModule`。
*   **實作**：使用本地的小型 Embedding 模型（如 `BGE-M3`）搭配 SQLite-VSS。
*   **價值**：讓所有 Agent 在處理任務前都能精確檢索沙盒內的歷史程式碼與文件。

## 3. 多代理人「辯論」模式 (Agent Debate/Review Pipeline)
*   **構想**：不只由一個 Agent 執行，而是經過複數 Agent 的檢核。
*   **場景**：路徑配置為 `["Gemini_Generator", "Claude_Reviewer", "Aider_Implementer"]`。
*   **價值**：發揮不同 LLM 的強項（Gemini 擅長創意/廣度，Claude 擅長程式邏輯/嚴謹度），提升最終產出品質。

## 4. 動態流水線視覺化監控 (Pipeline Visualizer)
*   **構想**：實作一個即時儀表板，監控任務在模組間的移動。
*   **實作**：透過 WebSocket 將 `history` 更新推送到管理界面。
*   **價值**：對於長達數小時的複雜任務，視覺化能幫助快速診斷瓶頸發生在哪個模組（例如某個 Agent 執行過久）。

## 5. 資源限制與隔離 (Resource Isolation Plus)
*   **構想**：針對腳本執行（Lua/Python）限制 CPU 與記憶體使用量。
*   **實作**：在 Linux 下使用 `cgroups`，在 Windows 下使用 `Job Objects` 封裝執行序。
*   **價值**：確保單一惡意或死迴圈腳本不會拖慢整個伺服器的隊列處理能力。

---
*這些進階功能旨在將伺服器從一個單純的「轉發中心」提升為「智慧型協作環境」。*
