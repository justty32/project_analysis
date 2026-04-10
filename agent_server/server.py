"""AI Agent Server — FastAPI 入口。"""

from __future__ import annotations

import asyncio
import logging
from contextlib import asynccontextmanager

import httpx
import uvicorn
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel

from core.orchestrator import Orchestrator
from core.registry import ModuleRegistry
from core.router import Router
from models.envelope import TaskEnvelope, TaskStatus
from modules.ollama_connector import OllamaConnector
from storage.sqlite_store import SQLiteStore

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("agent_server")

# ---------------------------------------------------------------------------
# 全域物件（在 lifespan 中初始化）
# ---------------------------------------------------------------------------
store: SQLiteStore
orchestrator: Orchestrator
registry: ModuleRegistry

# 追蹤背景非同步任務，避免被 GC 回收
_background_tasks: dict[str, asyncio.Task] = {}


@asynccontextmanager
async def lifespan(app: FastAPI):
    global store, orchestrator, registry

    # HTTP client
    http_client = httpx.AsyncClient(timeout=60)
    app.state.http_client = http_client

    # 模組註冊
    ollama = OllamaConnector(http_client=http_client)
    registry = ModuleRegistry()
    registry.register(ollama)

    # Router + Orchestrator
    router = Router.from_yaml()
    orchestrator = Orchestrator(router=router, registry=registry)

    # SQLite
    store = SQLiteStore()

    logger.info("Agent Server started — modules: %s", registry.list_modules())
    yield

    await http_client.aclose()
    store.close()
    logger.info("Agent Server shut down")


app = FastAPI(title="AI Agent Server", lifespan=lifespan)


# ---------------------------------------------------------------------------
# Request / Response models
# ---------------------------------------------------------------------------
class ChatRequest(BaseModel):
    Content: str
    Title: str = "default"
    Metadata: dict = {}


class TaskCreateRequest(BaseModel):
    content: str
    tags: dict[str, str] = {}
    metadata: dict = {}


# ---------------------------------------------------------------------------
# /chat — 同步對話端點（向下相容原有介面）
# ---------------------------------------------------------------------------
@app.post("/chat")
async def chat_endpoint(request: ChatRequest):
    envelope = TaskEnvelope(
        content=request.Content,
        tags={"op_type": "chat", "provider": "ollama"},
        metadata={"title": request.Title, **request.Metadata},
    )

    envelope = await orchestrator.run(envelope)
    store.save(envelope)

    if envelope.status == TaskStatus.FAILED:
        raise HTTPException(status_code=500, detail=envelope.error)

    return {
        "type": "ChatReply",
        "Title": request.Title,
        "Content": {"full_response": envelope.content},
    }


# ---------------------------------------------------------------------------
# /tasks — 非同步任務 CRUD
# ---------------------------------------------------------------------------
@app.post("/tasks", status_code=201)
async def create_task(request: TaskCreateRequest):
    envelope = TaskEnvelope(
        content=request.content,
        tags=request.tags,
        metadata=request.metadata,
    )
    store.save(envelope)

    async def _run(env: TaskEnvelope) -> None:
        result = await orchestrator.run(env)
        store.save(result)
        _background_tasks.pop(env.id, None)

    task = asyncio.create_task(_run(envelope))
    _background_tasks[envelope.id] = task

    return {"task_id": envelope.id, "status": envelope.status.value}


@app.get("/tasks/{task_id}")
async def get_task(task_id: str):
    envelope = store.get(task_id)
    if envelope is None:
        raise HTTPException(status_code=404, detail="Task not found")
    return envelope.model_dump()


@app.delete("/tasks/{task_id}")
async def cancel_task(task_id: str):
    envelope = store.get(task_id)
    if envelope is None:
        raise HTTPException(status_code=404, detail="Task not found")

    if envelope.status in (TaskStatus.COMPLETED, TaskStatus.FAILED, TaskStatus.CANCELLED):
        raise HTTPException(status_code=409, detail=f"Task already {envelope.status.value}")

    envelope.transition(TaskStatus.CANCELLED)
    store.save(envelope)

    bg = _background_tasks.pop(task_id, None)
    if bg and not bg.done():
        bg.cancel()

    return {"task_id": task_id, "status": "cancelled"}


# ---------------------------------------------------------------------------
# /health — 健康檢查
# ---------------------------------------------------------------------------
@app.get("/health")
async def health():
    ollama_ok = False
    try:
        resp = await app.state.http_client.get("http://localhost:11434/api/tags")
        ollama_ok = resp.status_code == 200
    except Exception:
        pass

    return {
        "status": "ok",
        "ollama": "connected" if ollama_ok else "disconnected",
        "modules": registry.list_modules(),
    }


if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
