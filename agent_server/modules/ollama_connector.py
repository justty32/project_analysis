"""OllamaConnector — 本地 Ollama API 呼叫模組。"""

from __future__ import annotations

import logging
import os

import httpx

from models.envelope import TaskEnvelope, TaskStatus
from modules.base import BaseModule

logger = logging.getLogger(__name__)

OLLAMA_BASE_URL = os.getenv("OLLAMA_BASE_URL", "http://localhost:11434")
DEFAULT_MODEL = os.getenv("DEFAULT_MODEL", "gemma3:12b")


class OllamaConnector(BaseModule):
    @property
    def name(self) -> str:
        return "ollama_connector"

    def __init__(self, http_client: httpx.AsyncClient | None = None) -> None:
        self._client = http_client

    def set_client(self, client: httpx.AsyncClient) -> None:
        self._client = client

    async def process(self, envelope: TaskEnvelope) -> TaskEnvelope:
        if self._client is None:
            raise RuntimeError("HTTP client not set on OllamaConnector")

        model = envelope.tags.get("model", DEFAULT_MODEL)
        payload = {
            "model": model,
            "messages": [{"role": "user", "content": envelope.content}],
            "stream": False,
        }

        try:
            resp = await self._client.post(
                f"{OLLAMA_BASE_URL}/api/chat", json=payload
            )
            resp.raise_for_status()
            result = resp.json()
            envelope.content = result.get("message", {}).get("content", "")
            envelope.transition(TaskStatus.COMPLETED)
        except httpx.HTTPError as exc:
            envelope.transition(TaskStatus.FAILED)
            envelope.error = f"Ollama error: {exc}"
            logger.error("Ollama call failed: %s", exc)

        return envelope
