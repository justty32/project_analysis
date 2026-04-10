# Millénaire Reborn - Gemini CLI Context

This project is a modern port of the [**Millénaire**](https://www.curseforge.com/minecraft/mc-mods/millenaire) mod for Minecraft 1.21.8 using the Fabric Loader. It aims to restore the immersive 1000 AD culture NPC villages with a clean, modern codebase.

## 🏗️ Project Architecture

The mod follows a standard Fabric structure with split environment source sets:
- `src/main/java/me/devupdates/millenaireReborn/`: Main mod package.
- `src/main/java/me/devupdates/millenaireReborn/common/`: Shared logic (registries, config, networking, utilities).
- `src/client/java/me/devupdates/millenaireReborn/client/`: Client-only logic (rendering, input, UI).
- `OldSource/`: Contains the original 1.12.2 Forge codebase for reference during porting.

### Key Components
- **Registry System:** Centralized in `me.devupdates.millenaireReborn.common.registry`.
  - `MillItems`: Handles registration of 163+ cultural items.
  - `MillBlocks`: Currently contains basic blocks; expansion is ongoing.
  - `MillCreativeTabs`: Custom tabs for each culture (Norman, Byzantine, etc.).
  - `MillCustomMaterials`: Defines tool and armor materials.
- **Asset Pipeline:** Data generation is used for blockstates, models, and translations.
- **Networking:** Foundation implemented in `me.devupdates.millenaireReborn.common.network`.

## 🛠️ Development Workflow

### Building and Running
- **Build Mod:** `./gradlew build`
- **Run Client:** `./gradlew runClient`
- **Run Server:** `./gradlew runServer`
- **Generate Assets:** `./gradlew runDatagen`

### Coding Conventions
- **Language:** Java 21.
- **Registry:** Always use `MillRegistry` methods for registering new content to ensure consistency.
- **Localization:** Use the data generator for generating language files when possible, otherwise update `src/main/resources/assets/millenaire-reborn/lang/`.
- **Porting:** Reference `OldSource` for original logic and data, but adapt to modern Minecraft/Fabric APIs (e.g., using `RegistryKey` for items in 1.21.8).

## 🚀 Current Roadmap (Status: Phase 3)
- [x] **Phase 1 & 2:** Infrastructure, 163 Cultural Items, Creative Tabs, Localization.
- [🚧] **Phase 3:** Advanced Item Functionality (Tools, Armor, Food effects, Bows).
- [📅] **Phase 4:** Comprehensive Block Porting (~200 blocks).
- [📅] **Phase 5-7:** Entities, AI, Village Generation, Trading, and Quests.

## 📖 Key Documentation
- `README.md`: Overview and installation.
- `ROADMAP.md`: Detailed development phases and progress tracking.
