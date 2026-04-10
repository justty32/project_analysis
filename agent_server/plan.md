# AI Agent Server 實作計畫

## 1. 核心目標

- **標籤驅動路由**：模組間透過 Task Envelope 的 `tags` 動態決定處理路徑。
- **多模型整合**：支援 Ollama (Local) 與 Gemini API (Cloud)。
- **跨平台沙盒執行**：Windows (Job Objects) / Linux (cgroups) / Docker。
- **任務生命週期管理**：Watchdog 超時控制，SQLite 持久化。
- **全流程可觀測性**：Trace ID 追蹤與結構化日誌。

## 2. 階段性實作規劃

### 第零階段：專案基礎建設

> 所有後續階段的前置條件。

- [ ] **建立 `pyproject.toml`**：定義依賴（fastapi, uvicorn, httpx, pydantic）
- [ ] **建立專案結構**：
    ```
    agent_server/
    ├── server.py              # FastAPI app 入口
    ├── models/
    │   └── envelope.py        # TaskEnvelope 定義
    ├── core/
    │   ├── orchestrator.py    # 協調器
    │   ├── router.py          # 路由器
    │   ├── registry.py        # 模組註冊表
    │   └── watchdog.py        # 超時監控
    ├── modules/
    │   ├── base.py            # BaseModule ABC
    │   ├── ollama_connector.py
    │   └── ...
    ├── storage/
    │   └── sqlite_store.py    # 任務持久化
    ├── config/
    │   └── routes.yaml        # 路由規則設定
    └── workspace/             # 沙盒工作目錄
    ```
- [ ] **設定外部化**：環境變數 + `.env` 檔，搭配 `pydantic-settings`

### 第一階段：流水線核心

> 依賴：第零階段完成。

- [ ] **實作 `TaskEnvelope` 資料模型**（`models/envelope.py`）
    - 對應 `architecture.md` 第 2 節的完整欄位定義
    - 含狀態機轉換的驗證邏輯
- [ ] **實作 `BaseModule` 抽象介面**（`modules/base.py`）
    - `name` 屬性 + `async def process(envelope) -> envelope`
- [ ] **實作 `ModuleRegistry`**（`core/registry.py`）
    - `register(module)` / `get(name) -> module`
- [ ] **實作 `Router`**（`core/router.py`）
    - 從 `config/routes.yaml` 載入路由規則
    - AND 匹配邏輯，優先匹配最具體的規則
- [ ] **實作 `Orchestrator`**（`core/orchestrator.py`）
    - 接收 Envelope → Router 取得模組 → 呼叫 → 更新 → 迴圈
    - 捕獲模組未預期例外，標記 `failed`
- [ ] **實作 `OllamaConnector`**（`modules/ollama_connector.py`）
    - 將現有 `server.py` 的 Ollama 呼叫邏輯遷移為模組
- [ ] **更新 `server.py` API 端點**
    - `/chat`：透過 Orchestrator 處理（同步等待結果）
    - `/health`：回傳 server 與 Ollama 連線狀態

### 第二階段：非同步任務與擴充模組

> 依賴：第一階段的 Orchestrator 和 Router 可運作。

- [ ] **實作 SQLite 持久化**（`storage/sqlite_store.py`）
    - 儲存 / 更新 / 查詢 Task Envelope
- [ ] **實作非同步任務 API**
    - `POST /tasks`：建立 Envelope，`asyncio.create_task` 啟動 Orchestrator，立即回傳 `task_id`
    - `GET /tasks/{id}`：從 SQLite 查詢狀態與結果
    - `DELETE /tasks/{id}`：標記 `cancelled`，通知 Orchestrator 中止
- [ ] **實作 Watchdog**（`core/watchdog.py`）
    - 背景任務，定期掃描 running 狀態的 Envelope
    - 超時值從 `envelope.metadata.timeout` 讀取（唯一來源）
    - 超時後標記 `failed` 並記錄原因
- [ ] **實作 `GeminiConnector`**（`modules/gemini_connector.py`）
    - Google Gemini API 呼叫，需處理 API Key 認證
- [ ] **實作 Agent Wrapper 基礎**
    - `claude_wrapper`：透過 subprocess 啟動 Claude CLI
    - 強制 `workspace/` 路徑邊界檢查

### 第三階段：沙盒執行與錯誤處理

> 依賴：第二階段的持久化與 Watchdog 可運作。

- [ ] **實作跨平台腳本沙盒**
    - `SandboxFactory`：偵測平台，回傳對應的隔離策略
    - Windows：`subprocess` + Job Objects
    - Linux：`subprocess` + cgroups
    - 限制 CPU 時間、記憶體用量、檔案系統存取範圍
- [ ] **實作 `PythonExecutor` / `LuaExecutor`**
    - 在沙盒內執行使用者提供的腳本
    - 結果回填至 `envelope.content`
- [ ] **實作錯誤處理機制**
    - 失敗的 Envelope 記錄至 `failed_tasks` 表（Dead Letter Queue）
    - 提供 `GET /tasks?status=failed` 查詢介面
- [ ] **實作結果回調（選用）**
    - 支援 `metadata.callback_url`，任務完成時 POST 通知

## 3. 技術決策摘要

| 決策項目       | 選擇                | 理由                                   |
| -------------- | ------------------- | -------------------------------------- |
| HTTP Client    | `httpx` (async)     | 不阻塞 event loop                      |
| 持久化         | SQLite              | 零依賴，單檔部署，足夠應付單機工作量     |
| 設定管理       | 環境變數 + .env     | 12-factor app 原則                     |
| 路由設定       | YAML                | 可讀性好，支援動態載入                   |
| 腳本沙盒       | 平台偵測 + 抽象工廠  | 開發環境 Windows，生產可切換至 Docker    |
| 並發模型       | asyncio.create_task | 每個 Task 獨立執行，互不阻塞             |

## 4. 階段依賴關係

```
第零階段（基礎建設）
    │
    ▼
第一階段（流水線核心）
    │
    ▼
第二階段（非同步任務 + 擴充模組）
    │
    ▼
第三階段（沙盒 + 錯誤處理）
```

每個階段完成後應可獨立運行與測試。第一階段完成時，系統應能透過 `/chat` 正常對話（功能與當前相同，但內部已走 Orchestrator 流水線）。
