# 如何添加自定義模組

本文說明如何為 Agent Server 添加一個新的執行模組。整個流程只需三步：**寫模組 → 加路由 → 註冊**。

---

## 概念

每個模組都是一個 `BaseModule` 的子類別，只需要實作兩件事：

- **`name`** — 唯一名稱（字串），Router 和 history 都靠它識別你的模組
- **`process(envelope)`** — 接收一個 `TaskEnvelope`，處理後回傳

Orchestrator 會根據 envelope 裡的 `tags` 透過 Router 找到你的模組，呼叫 `process`，然後把結果繼續往下傳。

---

## 第一步：寫模組

在 `modules/` 下建立新檔案，例如 `modules/my_module.py`：

```python
from __future__ import annotations

import logging

from models.envelope import TaskEnvelope, TaskStatus
from modules.base import BaseModule

logger = logging.getLogger(__name__)


class MyModule(BaseModule):
    @property
    def name(self) -> str:
        return "my_module"  # 唯一名稱，要跟路由規則裡的 module 值一致

    async def process(self, envelope: TaskEnvelope) -> TaskEnvelope:
        # 1. 從 envelope 讀取輸入
        user_input = envelope.content
        some_option = envelope.tags.get("my_option", "default")

        # 2. 做你想做的事
        result = f"Processed: {user_input} (option={some_option})"

        # 3. 把結果寫回 envelope
        envelope.content = result

        # 4. 設定狀態
        #    - 處理完成：transition 到 COMPLETED
        #    - 處理失敗：transition 到 FAILED 並填入 error
        #    - 需要後續模組接手：不要 transition，改寫 tags 讓 Router 導到下一個模組
        envelope.transition(TaskStatus.COMPLETED)

        return envelope
```

### 關鍵規則

| 情境 | 怎麼做 |
|------|--------|
| 處理成功，任務結束 | `envelope.transition(TaskStatus.COMPLETED)` |
| 處理失敗（可預期的錯誤） | `envelope.transition(TaskStatus.FAILED)` + 填 `envelope.error` |
| 未預期的例外 | 直接 `raise`，Orchestrator 會自動捕獲並標記 `failed` |
| 需要交給下一個模組 | 修改 `envelope.tags`（例如改 `op_type` 或設 `next_module`） |

### 使用 `next_module` 做顯式串接

如果你想讓模組 A 處理完後直接交給模組 B，不經過路由規則匹配：

```python
async def process(self, envelope: TaskEnvelope) -> TaskEnvelope:
    # ... 處理邏輯 ...

    # 直接指定下一個模組（Router 會優先使用這個值）
    envelope.tags["next_module"] = "module_b"
    return envelope
```

> Orchestrator 每輪結束會自動清除 `next_module`，所以不會無限迴圈。

---

## 第二步：加路由規則

編輯 `config/routes.yaml`，告訴 Router 什麼條件下要使用你的模組：

```yaml
routes:
  # ... 原有規則 ...

  # 新增：當 op_type 是 my_task 時，路由到 my_module
  - match:
      op_type: my_task
    module: my_module
```

### 匹配邏輯

- **AND 匹配**：`match` 裡的所有條件必須同時滿足
- **最具體優先**：條件越多的規則優先級越高
- 條件對應的是 `envelope.tags` 裡的鍵值對

例如，這兩條規則：

```yaml
  - match:
      op_type: chat          # 1 個條件
    module: ollama_connector

  - match:
      op_type: chat           # 2 個條件 → 更具體 → 優先
      provider: gemini
    module: gemini_connector
```

當 tags 為 `{op_type: chat, provider: gemini}` 時，會匹配到 `gemini_connector`（因為更具體）。

---

## 第三步：註冊模組

編輯 `server.py` 的 `lifespan` 函式，把你的模組實例化並註冊：

```python
from modules.my_module import MyModule       # 1. import

@asynccontextmanager
async def lifespan(app: FastAPI):
    global store, orchestrator, registry

    http_client = httpx.AsyncClient(timeout=60)
    app.state.http_client = http_client

    # 模組註冊
    ollama = OllamaConnector(http_client=http_client)
    my_mod = MyModule()                       # 2. 實例化

    registry = ModuleRegistry()
    registry.register(ollama)
    registry.register(my_mod)                 # 3. 註冊

    # ... 後續不變 ...
```

完成後重啟 server，訪問 `/health` 就能看到新模組出現在 `modules` 清單中。

---

## 呼叫你的模組

### 同步（/chat 端點不適用，它固定走 ollama）

### 非同步（透過 /tasks）

```bash
curl -X POST http://localhost:8000/tasks \
  -H "Content-Type: application/json" \
  -d '{
    "content": "要處理的內容",
    "tags": {"op_type": "my_task"}
  }'
```

回傳 `task_id` 後，用 `GET /tasks/{task_id}` 查詢結果。

---

## 完整範例：一個翻譯模組

```python
# modules/translator.py
from __future__ import annotations

import httpx

from models.envelope import TaskEnvelope, TaskStatus
from modules.base import BaseModule


class Translator(BaseModule):
    @property
    def name(self) -> str:
        return "translator"

    def __init__(self, http_client: httpx.AsyncClient) -> None:
        self._client = http_client

    async def process(self, envelope: TaskEnvelope) -> TaskEnvelope:
        target_lang = envelope.tags.get("target_lang", "en")

        # 用 Ollama 做翻譯
        resp = await self._client.post(
            "http://localhost:11434/api/chat",
            json={
                "model": "gemma3:12b",
                "messages": [
                    {"role": "user", "content": f"Translate to {target_lang}: {envelope.content}"}
                ],
                "stream": False,
            },
        )
        resp.raise_for_status()
        envelope.content = resp.json()["message"]["content"]
        envelope.transition(TaskStatus.COMPLETED)
        return envelope
```

路由：

```yaml
  - match:
      op_type: translate
    module: translator
```

註冊：

```python
translator = Translator(http_client=http_client)
registry.register(translator)
```

使用：

```bash
curl -X POST http://localhost:8000/tasks \
  -d '{"content": "你好世界", "tags": {"op_type": "translate", "target_lang": "ja"}}'
```
