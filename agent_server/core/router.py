"""Router — 根據 Envelope 的 tags 決定下一個處理模組。"""

from __future__ import annotations

import logging
from pathlib import Path
from typing import Any

import yaml

from models.envelope import TaskEnvelope

logger = logging.getLogger(__name__)

# 預設路由設定檔路徑
_DEFAULT_ROUTES_PATH = Path(__file__).resolve().parent.parent / "config" / "routes.yaml"


class RouteRule:
    """單一路由規則：match 條件 + 目標 module 名稱。"""

    def __init__(self, match: dict[str, str], module: str) -> None:
        self.match = match
        self.module = module

    def matches(self, tags: dict[str, str]) -> bool:
        """所有 match 條件必須同時滿足（AND 邏輯）。"""
        return all(tags.get(k) == v for k, v in self.match.items())

    @property
    def specificity(self) -> int:
        """條件數量越多越具體。"""
        return len(self.match)


class Router:
    def __init__(self, rules: list[RouteRule] | None = None) -> None:
        self._rules: list[RouteRule] = rules or []

    @classmethod
    def from_yaml(cls, path: Path | str | None = None) -> Router:
        """從 YAML 設定檔載入路由規則。"""
        path = Path(path) if path else _DEFAULT_ROUTES_PATH
        with open(path, encoding="utf-8") as f:
            data: dict[str, Any] = yaml.safe_load(f)

        rules = [
            RouteRule(match=entry["match"], module=entry["module"])
            for entry in data.get("routes", [])
        ]
        logger.info("Loaded %d route rules from %s", len(rules), path)
        return cls(rules)

    def resolve(self, envelope: TaskEnvelope) -> str | None:
        """回傳最具體的匹配模組名稱，若有 next_module tag 則優先使用。"""
        # next_module 直接覆寫路由
        if next_mod := envelope.tags.get("next_module"):
            return next_mod

        matched: list[RouteRule] = [r for r in self._rules if r.matches(envelope.tags)]
        if not matched:
            return None

        # 最具體優先
        matched.sort(key=lambda r: r.specificity, reverse=True)
        return matched[0].module
