"""Orchestrator — 維護 Task Envelope 的狀態機與生命週期。"""

from __future__ import annotations

import logging

from core.registry import ModuleRegistry
from core.router import Router
from models.envelope import TaskEnvelope, TaskStatus

logger = logging.getLogger(__name__)


class Orchestrator:
    def __init__(self, router: Router, registry: ModuleRegistry) -> None:
        self.router = router
        self.registry = registry

    async def run(self, envelope: TaskEnvelope) -> TaskEnvelope:
        """
        主迴圈：Router 取得模組 → 呼叫 → 更新 → 迴圈直到終止。

        終止條件：
        - status 變為 completed / failed / cancelled
        - 沒有下一個模組可路由（隱式完成）
        """
        trace = f"[{envelope.id[:8]}]"
        envelope.transition(TaskStatus.RUNNING)
        logger.info("%s Task started", trace)

        while envelope.status == TaskStatus.RUNNING:
            module_name = self.router.resolve(envelope)

            if module_name is None:
                # 隱式完成
                envelope.transition(TaskStatus.COMPLETED)
                logger.info("%s No more modules, task completed", trace)
                break

            try:
                module = self.registry.get(module_name)
            except KeyError:
                envelope.transition(TaskStatus.FAILED)
                envelope.error = f"Module not found: {module_name}"
                envelope.add_history("orchestrator", "failed", envelope.error)
                logger.error("%s %s", trace, envelope.error)
                break

            logger.info("%s Dispatching to module: %s", trace, module_name)

            try:
                envelope = await module.process(envelope)
                envelope.add_history(module_name, envelope.status.value)
            except Exception as exc:
                envelope.transition(TaskStatus.FAILED)
                envelope.error = f"Unhandled exception in {module_name}: {exc}"
                envelope.add_history(module_name, "failed", str(exc))
                logger.exception("%s Module %s raised", trace, module_name)
                break

            # 清除 next_module 以免無限迴圈（模組若需要可重新設定）
            envelope.tags.pop("next_module", None)

        return envelope
