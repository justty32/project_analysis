# Veloren Project Instructional Context (GEMINI.md)

Welcome to the **Veloren** project. This file provides high-level architectural overview, development commands, and **MANDATORY** interaction protocols for Gemini CLI.

## 核心行為規範 (Mandatory Protocols)

- **自動留檔要求**：針對此專案的所有分析、拆解、詢問回答、教程撰寫，**必須同步留檔**於 `work/` 資料夾下的對應子目錄中。
- **輸出語言**：所有留檔內容與對話回應必須使用 **繁體中文**。
- **目錄結構規範**：
    - `work/architecture/`: 架構分析報告。
    - `work/tutorial/`: 開發與使用教程。
    - `work/analysis/`: 深度原始碼拆解紀錄。
    - `work/answer/`: 具體問題的詳細解答。
    - `work/detail/`: 系統細節描述。
    - `work/other/`: 其他雜項（如 SOP、腳本）。
    - `work/gemini_temp/`: 會話進度保存與暫存檔。
- **日誌追蹤**：必須維持 `work/session_log.md` 的更新，記錄每一次要求的具體執行事項。
- **程式碼標註**：所有在分析或教程中提到的程式碼，必須標註其原始碼位置（檔案路徑與大約行號或函數名）。

## Project Overview

**Veloren** is a multi-player voxel RPG written in **Rust**. It uses an **ECS (Entity Component System)** architecture powered by the `specs` crate.

- **Main Technologies:** Rust (2021/2024 edition), `specs` (ECS), `wgpu` (Graphics), `egui` (UI), `quinn` (QUIC networking), `rusqlite` (Persistence), and `ron` (Configuration/Assets).

## Building and Running

- **Run Client:** `cargo run --bin veloren-voxygen`
- **Run Server:** `cargo run --bin veloren-server`
- **Test Workspace:** `cargo test --workspace`
- **Release Build:** `cargo build --release`

## Development Conventions

- **Crucial Rule:** **NO AI-generated content** (code or assets) is allowed in the official repository (per `CONTRIBUTING.md`).
- **Asset Management:** Assets are defined in `.ron` files. See `assets/` and `common/assets/`.

---
*This file was updated by Gemini CLI to strictly enforce the archiving and language protocols.*
