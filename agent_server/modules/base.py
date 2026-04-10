"""BaseModule — 所有執行模組的抽象基底類別。"""

from __future__ import annotations

from abc import ABC, abstractmethod

from models.envelope import TaskEnvelope


class BaseModule(ABC):
    """每個模組須實作 name 屬性與 process 方法。"""

    @property
    @abstractmethod
    def name(self) -> str:
        """唯一名稱，用於 Router 路由與 history 記錄。"""
        ...

    @abstractmethod
    async def process(self, envelope: TaskEnvelope) -> TaskEnvelope:
        """處理 envelope 並回傳更新後的版本。"""
        ...
