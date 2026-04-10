"""Task Envelope — 系統內所有模組間傳遞的唯一資料結構。"""

from __future__ import annotations

import uuid
from datetime import datetime, timezone
from enum import Enum
from typing import Any

from pydantic import BaseModel, Field


class TaskStatus(str, Enum):
    PENDING = "pending"
    RUNNING = "running"
    COMPLETED = "completed"
    FAILED = "failed"
    CANCELLED = "cancelled"
    AWAITING_USER_APPROVAL = "awaiting_user_approval"


# 狀態機：定義每個狀態允許的下一步轉換
_VALID_TRANSITIONS: dict[TaskStatus, set[TaskStatus]] = {
    TaskStatus.PENDING: {TaskStatus.RUNNING, TaskStatus.CANCELLED},
    TaskStatus.RUNNING: {
        TaskStatus.COMPLETED,
        TaskStatus.FAILED,
        TaskStatus.CANCELLED,
        TaskStatus.AWAITING_USER_APPROVAL,
    },
    TaskStatus.AWAITING_USER_APPROVAL: {TaskStatus.RUNNING, TaskStatus.CANCELLED},
    TaskStatus.COMPLETED: set(),
    TaskStatus.FAILED: set(),
    TaskStatus.CANCELLED: set(),
}


class HistoryEntry(BaseModel):
    module: str
    timestamp: str = Field(default_factory=lambda: datetime.now(timezone.utc).isoformat())
    status: str
    summary: str = ""


class TaskEnvelope(BaseModel):
    id: str = Field(default_factory=lambda: uuid.uuid4().hex)
    content: str = ""
    tags: dict[str, str] = Field(default_factory=dict)
    metadata: dict[str, Any] = Field(default_factory=dict)
    history: list[HistoryEntry] = Field(default_factory=list)
    status: TaskStatus = TaskStatus.PENDING
    error: str | None = None

    def transition(self, new_status: TaskStatus) -> None:
        """執行狀態轉換，若不合法則拋出 ValueError。"""
        allowed = _VALID_TRANSITIONS.get(self.status, set())
        if new_status not in allowed:
            raise ValueError(
                f"Invalid transition: {self.status.value} -> {new_status.value}"
            )
        self.status = new_status

    def add_history(self, module: str, status: str, summary: str = "") -> None:
        """記錄一筆處理軌跡。"""
        self.history.append(HistoryEntry(module=module, status=status, summary=summary))
