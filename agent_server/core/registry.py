"""ModuleRegistry — 模組註冊表，啟動時註冊，Router 透過 name 取得實例。"""

from __future__ import annotations

from modules.base import BaseModule


class ModuleRegistry:
    def __init__(self) -> None:
        self._modules: dict[str, BaseModule] = {}

    def register(self, module: BaseModule) -> None:
        """註冊模組，名稱重複時拋出 ValueError。"""
        if module.name in self._modules:
            raise ValueError(f"Module already registered: {module.name}")
        self._modules[module.name] = module

    def get(self, name: str) -> BaseModule:
        """根據名稱取得模組，找不到時拋出 KeyError。"""
        try:
            return self._modules[name]
        except KeyError:
            raise KeyError(f"Module not found: {name}")

    def list_modules(self) -> list[str]:
        return list(self._modules.keys())
