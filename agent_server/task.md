# 骨架建置任務清單

> 接手時說「繼續做骨架」即可。

## 已完成
- [x] 建立目錄結構（models/, core/, modules/, storage/, config/, workspace/）
- [x] 建立四個 `__init__.py`
- [x] `models/envelope.py` — TaskEnvelope Pydantic model，含狀態機轉換、history 記錄邏輯
- [x] `modules/base.py` + `core/registry.py` — BaseModule ABC、ModuleRegistry
- [x] `core/router.py` — YAML 路由規則載入，AND 匹配，最具體優先
- [x] `core/orchestrator.py` — 主迴圈：Router → 模組 → 更新 → 迴圈，例外捕獲
- [x] `modules/ollama_connector.py` — Ollama 呼叫邏輯，實作 BaseModule
- [x] `storage/sqlite_store.py` — SQLite CRUD（save/get/list_by_status）
- [x] `config/routes.yaml` + `pyproject.toml` — 預設路由規則、專案依賴
- [x] 重構 `server.py` — 接入 Orchestrator，新增 `/tasks`（POST/GET/DELETE）與 `/health`

## 待做
（第一階段骨架已全部完成，進入第二階段前可先做整合測試）
