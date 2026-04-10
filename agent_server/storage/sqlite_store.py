"""SQLite Store — Task Envelope 持久化層。"""

from __future__ import annotations

import json
import logging
import sqlite3
from pathlib import Path

from models.envelope import TaskEnvelope

logger = logging.getLogger(__name__)

_DEFAULT_DB_PATH = Path(__file__).resolve().parent.parent / "data" / "tasks.db"

_CREATE_TABLE = """
CREATE TABLE IF NOT EXISTS tasks (
    id          TEXT PRIMARY KEY,
    status      TEXT NOT NULL,
    content     TEXT NOT NULL DEFAULT '',
    tags        TEXT NOT NULL DEFAULT '{}',
    metadata    TEXT NOT NULL DEFAULT '{}',
    history     TEXT NOT NULL DEFAULT '[]',
    error       TEXT,
    created_at  TEXT NOT NULL DEFAULT (datetime('now')),
    updated_at  TEXT NOT NULL DEFAULT (datetime('now'))
);
"""


class SQLiteStore:
    def __init__(self, db_path: Path | str | None = None) -> None:
        self._db_path = str(db_path or _DEFAULT_DB_PATH)
        Path(self._db_path).parent.mkdir(parents=True, exist_ok=True)
        self._conn = sqlite3.connect(self._db_path)
        self._conn.row_factory = sqlite3.Row
        self._conn.execute(_CREATE_TABLE)
        self._conn.commit()
        logger.info("SQLite store initialised at %s", self._db_path)

    def save(self, envelope: TaskEnvelope) -> None:
        """插入或更新一筆 Task Envelope。"""
        self._conn.execute(
            """
            INSERT INTO tasks (id, status, content, tags, metadata, history, error)
            VALUES (?, ?, ?, ?, ?, ?, ?)
            ON CONFLICT(id) DO UPDATE SET
                status     = excluded.status,
                content    = excluded.content,
                tags       = excluded.tags,
                metadata   = excluded.metadata,
                history    = excluded.history,
                error      = excluded.error,
                updated_at = datetime('now')
            """,
            (
                envelope.id,
                envelope.status.value,
                envelope.content,
                json.dumps(envelope.tags),
                json.dumps(envelope.metadata),
                json.dumps([h.model_dump() for h in envelope.history]),
                envelope.error,
            ),
        )
        self._conn.commit()

    def get(self, task_id: str) -> TaskEnvelope | None:
        """根據 ID 查詢 Task Envelope。"""
        row = self._conn.execute("SELECT * FROM tasks WHERE id = ?", (task_id,)).fetchone()
        if row is None:
            return None
        return self._row_to_envelope(row)

    def list_by_status(self, status: str) -> list[TaskEnvelope]:
        """查詢指定狀態的所有 Task Envelope。"""
        rows = self._conn.execute(
            "SELECT * FROM tasks WHERE status = ? ORDER BY created_at DESC", (status,)
        ).fetchall()
        return [self._row_to_envelope(r) for r in rows]

    def list_running(self) -> list[TaskEnvelope]:
        """查詢所有 running 狀態的 Task（供 Watchdog 使用）。"""
        return self.list_by_status("running")

    @staticmethod
    def _row_to_envelope(row: sqlite3.Row) -> TaskEnvelope:
        return TaskEnvelope(
            id=row["id"],
            status=row["status"],
            content=row["content"],
            tags=json.loads(row["tags"]),
            metadata=json.loads(row["metadata"]),
            history=json.loads(row["history"]),
            error=row["error"],
        )

    def close(self) -> None:
        self._conn.close()
