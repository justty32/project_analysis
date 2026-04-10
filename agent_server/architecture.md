# AI Agent Server 系統架構設計

## 1. 系統概觀

AI Agent Server 是一個中心化的協調器 (Orchestrator)，整合多個 AI 模型與外部代理人工具，透過標籤驅動的動態路由機制處理任務，並確保執行環境的安全與可追蹤性。

## 2. 核心資料結構：Task Envelope

Task Envelope 是系統內所有模組間傳遞的唯一資料結構：

```python
class TaskEnvelope(BaseModel):
    id: str                # UUID，全域唯一追蹤碼 (Trace ID)
    content: str           # 目前處理中的文字內容
    tags: dict[str, str]   # 驅動路由的鍵值對
    metadata: dict         # 建立時間、擁有者、超時設定等
    history: list[dict]    # 每階段處理軌跡
    status: str            # pending | running | completed | failed | cancelled | awaiting_user_approval
    error: str | None      # 失敗時的錯誤訊息
```

**Tags 設計原則：**
- `provider`：指定模型連接器（`ollama`, `gemini`）
- `next_module`：指定下一個處理模組名稱
- `op_type`：操作類型（`chat`, `script_exec`, `agent_run`）

**Status 狀態機：**
```
pending → running → completed
                  → failed
         → cancelled
         → awaiting_user_approval → running
```

## 3. 核心組件

### 3.1 API 層

技術選型：FastAPI

**端點設計：**

| 方法     | 路徑              | 說明                         |
| -------- | ----------------- | ---------------------------- |
| `POST`   | `/chat`           | 同步對話（短任務，直接等回應） |
| `POST`   | `/tasks`          | 提交非同步任務（回傳 task ID） |
| `GET`    | `/tasks/{id}`     | 查詢任務狀態與結果            |
| `DELETE` | `/tasks/{id}`     | 取消進行中的任務              |
| `GET`    | `/health`         | 健康檢查（含 Ollama 連線狀態） |

**設計決策：**
- `/chat` 保留作為簡單同步代理，適合短對話場景。
- `/tasks` 系列用於長時間、多步驟的流水線任務。
- 兩者共用相同的 Orchestrator 核心，差異僅在於回傳方式（同步等待 vs. 輪詢）。

### 3.2 Orchestrator（協調器）

職責：維護 Task Envelope 的狀態機與生命週期。

```
收到 Envelope → 查詢 Router 取得下一模組 → 呼叫模組 → 更新 Envelope → 迴圈直到終止
```

**終止條件：**
- `status` 變為 `completed` / `failed` / `cancelled`
- 沒有下一個模組可路由（隱式完成）
- Watchdog 觸發超時

**並發模型：**
- 多個獨立 Task 之間：`asyncio.create_task` 各自獨立執行
- 單一 Task 內的多步驟：序列執行（一步完成後才決定下一步）
- 未來擴充（多代理人辯論）：單一 Task 內可 `asyncio.gather` 並行呼叫多個模組

### 3.3 Router（路由器）

根據 Envelope 的 `tags` 決定下一個處理模組。

**路由規則格式（YAML 設定檔）：**
```yaml
routes:
  - match:
      op_type: chat
      provider: ollama
    module: ollama_connector

  - match:
      op_type: script_exec
    module: python_executor

  - match:
      op_type: agent_run
      provider: claude
    module: claude_wrapper
```

**匹配邏輯：**
- 所有 `match` 條件必須同時滿足（AND）
- 優先匹配條件數量最多的規則（越具體越優先）
- 模組可在處理後修改 `tags`，從而改變後續路由方向

**動態路由範例：**
```
使用者輸入 → Router(op_type:chat) → OllamaConnector
→ LLM 判斷需要執行腳本 → 更新 tags(op_type:script_exec)
→ Router → PythonExecutor → 結果回填 content → 完成
```

### 3.4 執行模組

所有模組實作統一介面：

```python
class BaseModule(ABC):
    name: str  # 唯一名稱，用於 Router 路由與 history 記錄

    @abstractmethod
    async def process(self, envelope: TaskEnvelope) -> TaskEnvelope:
        """處理 envelope 並回傳更新後的版本。"""
        ...
```

**模組註冊機制：**
- 模組在啟動時透過 `ModuleRegistry.register(module)` 註冊。
- Router 透過 `ModuleRegistry.get(name)` 取得模組實例。

**錯誤處理約定：**
- 模組內的可預期錯誤：設定 `envelope.status = "failed"` 並填入 `envelope.error`，正常回傳。
- 模組內的未預期例外：由 Orchestrator 捕獲，自動標記 `failed` 並記錄到 `history`。

**模組分類：**

| 分類               | 模組名稱             | 說明                       |
| ------------------ | -------------------- | -------------------------- |
| Model Connector    | `ollama_connector`   | 本地 Ollama API 呼叫       |
| Model Connector    | `gemini_connector`   | Google Gemini Cloud API    |
| Agent Wrapper      | `claude_wrapper`     | 啟動 Claude CLI 並擷取結果  |
| Agent Wrapper      | `aider_wrapper`      | 啟動 aider 進行程式碼修改   |
| Script Executor    | `python_executor`    | 受限環境下執行 Python 腳本  |
| Script Executor    | `lua_executor`       | 受限環境下執行 Lua 腳本     |

### 3.5 沙盒與安全管理

**檔案系統隔離：**
- 所有檔案操作強制限制在 `workspace/` 目錄下。
- 使用 `pathlib.Path.resolve()` 進行邊界檢查，防止 Path Traversal。

**腳本執行隔離（跨平台策略）：**

| 平台    | 隔離方式                                           |
| ------- | -------------------------------------------------- |
| Windows | `subprocess` + Job Objects 限制 CPU / 記憶體 / 時間 |
| Linux   | `subprocess` + `cgroups` / `unshare`               |
| 通用    | Docker container（推薦用於生產環境）                 |

啟動時自動偵測平台，選擇對應的隔離策略。

**資源限制：**
- 由 Watchdog 統一管理，讀取 `envelope.metadata.timeout`（預設 300 秒，上限 86400 秒）。
- Watchdog 是唯一的超時權威來源；`tags` 不重複儲存 timeout 值。

### 3.6 持久化層

| 資料         | 儲存方式         | 說明                             |
| ------------ | ---------------- | -------------------------------- |
| Task Envelope | SQLite           | 含 status、history、最終結果      |
| 路由設定      | YAML 檔案        | `config/routes.yaml`             |
| 應用設定      | 環境變數 + .env  | API URL、模型名稱、連接埠等       |

SQLite 確保 server 重啟後任務狀態不遺失，且不需要額外的資料庫服務。

## 4. 資料流向

```
Client
  │
  ├─ POST /chat（短任務）──────────────────────┐
  │                                            │
  ├─ POST /tasks（長任務）→ 回傳 task_id        │
  │   └─ GET /tasks/{id} 輪詢結果              │
  │                                            ▼
  │                                    ┌──────────────┐
  │                                    │  Orchestrator │
  │                                    └──────┬───────┘
  │                                           │
  │                                    ┌──────▼───────┐
  │                                    │    Router     │
  │                                    └──────┬───────┘
  │                                           │
  │                              ┌────────────┼────────────┐
  │                              ▼            ▼            ▼
  │                        ┌──────────┐ ┌──────────┐ ┌──────────┐
  │                        │ Model    │ │ Agent    │ │ Script   │
  │                        │Connector │ │ Wrapper  │ │ Executor │
  │                        └──────────┘ └──────────┘ └──────────┘
  │                                           │
  │                                    ┌──────▼───────┐
  │                                    │   Persist    │
  │                                    │  (SQLite)    │
  │                                    └──────────────┘
  │                                           │
  ◄───────────────────────────────────────────┘
```

## 5. 可觀測性

- **Trace ID**：每個 Task Envelope 的 `id` 貫穿整個生命週期。
- **Centralized Logging**：所有模組使用 `logging` 模組，以 `[trace_id][module_name]` 為前綴。
- **History Trace**：`envelope.history` 記錄每步的模組名稱、時間戳、狀態與摘要。
