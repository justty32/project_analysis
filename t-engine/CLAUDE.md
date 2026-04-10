# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

T-Engine4 uses **Premake4** to generate platform-specific build files.

```bash
# Generate Makefiles (Linux/BSD)
premake4 gmake

# Generate Visual Studio project (Windows)
premake4 vs2010

# Build
make -C build

# Output binary: bin/Debug/t-engine (or bin/Release/t-engine)
```

Key build options (pass to `premake4`):
- `--lua=jit2` — Use LuaJIT 2.x (default); `--lua=default` for Lua 5.1
- `--steam` — Enable Steam integration
- `--discord` — Enable Discord Rich Presence
- `--web-cef3` — Enable embedded browser via CEF3
- `--force32bits` — Build 32-bit binary
- `--pedantic` — Stricter C compiler warnings

No test suite or linter exists.

## Architecture Overview

T-Engine4 is a **C + Lua hybrid roguelike engine**. The C layer provides performance-critical systems; all game logic and modding is in Lua.

### Layers

```
Game Modules (.team zip archives)       ← game content (e.g. Tales of Maj'Eyal)
Engine Core (.teae zip archive)         ← 200+ Lua modules (entity, UI, AI, etc.)
C ↔ Lua Bridge (core_lua.c)             ← exposes C subsystems as core.* Lua API
C Foundation (SDL2, OpenGL, PhysFS...)  ← platform, rendering, filesystem
```

### Key C Subsystems (`src/`)

| File | Role |
|------|------|
| `main.c` | SDL2 window/context, main loop, input |
| `core_lua.c` (106KB) | Central C→Lua bridge; registers all `core.*` Lua APIs |
| `map.c` | C-accelerated tile rendering and viewport |
| `fov.c` | Recursive shadowcasting FOV algorithms |
| `display_sdl.c` | FBO/texture/framebuffer management |
| `particles.c` | Hardware-accelerated particle system |
| `physfs.c` | Virtual filesystem (transparent zip access) |
| `serial.c` | Lua object serialization / save-load |
| `SFMT.c` | SIMD Mersenne Twister RNG |
| `noise.c` | Perlin/Simplex noise |
| `profile.c` | Online profiles, leaderboards, achievements |
| `src/wfc/` (C++) | Wave Function Collapse procedural generation |

### Virtual Filesystem

All file access goes through **PhysFS**. Archives (`.team`, `.teae`) are mounted to virtual paths like `/engines/`, `/modules/`, `/addons/`. Code reads resources the same way whether they're on disk or in zip archives.

### Bootstrap

`bootstrap/boot.lua` is the first Lua file executed. It mounts archives, selects the engine core version, and loads the appropriate game module.

---

## Lua Engine Core (`game/engines/te4-1.7.6.teae`)

### OOP System (`engine/class.lua`)

TE4 implements its own OOP on top of Lua 5.1:

```lua
module(..., package.seeall, class.make)               -- single/no inheritance
module(..., package.seeall, class.inherit(A, B, C))   -- multiple inheritance (mixin)
```

- `class.inherit` **copies** all base fields into the subclass (not `__index` chaining) for performance.
- Each instance has `__CLASSNAME` (string) used to reconstruct metatables during save/load.
- `__ATOMIC = true` marks a table as an object to prevent deep-copy during serialization.
- Hook/event system: `class:bindHook("Name", fn)` / `self:triggerHook{...}` — hooks marked `__persistent_hooks` survive save/load.

### Entity System

```
Entity (base: uid, attributes, define/resolve lifecycle)
  ├── Actor   (characters, monsters)
  ├── Grid    (terrain tiles)
  ├── Object  (items/equipment)
  ├── Trap
  └── Projectile
```

All entities have a unique `uid` and are registered in a global weak-value table `__uids` (GC'd when unreferenced).

**Define/Resolve lifecycle** — entities have two phases:
- **Prototype (define)**: attributes may contain `resolver` placeholders
- **Instance (resolve)**: `entity:resolve()` evaluates all resolvers to actual values

```lua
local proto = Actor.define{
    name = "Goblin",
    level = resolvers.rngrange(1, 5),       -- evaluated at resolve time
    talents = resolvers.talents{[T_ATTACK]=1},
}
local npc = proto:clone()
npc:resolve()  -- level becomes a real number, talents are learned
```

### Resolver System (`engine/resolvers.lua`)

Resolvers are lazy-evaluation placeholders. Structure: `{__resolver="foo", ...data}`.

| Resolver | Description |
|----------|-------------|
| `resolvers.rngrange(x, y)` | Random integer [x, y] |
| `resolvers.mbonus(max, add)` | Scales with current dungeon depth |
| `resolvers.talents(list)` | Learns talents on resolve |
| `resolvers.inventory(list)` | Generates items into backpack |
| `resolvers.equip(list)` | Generates and equips items |
| `resolvers.drops(list)` | Defines death drop table |

`resolvers.current_level` is set by Zone before generating entities so depth-scaling resolvers know the current floor.

### World Structure

```
World
 └─ Zone  (area, e.g. "Maze A")
     └─ Level  (floor, e.g. "B1F")
         ├─ Map  (2D grid of entities by Z-layer)
         └─ [Actor, ...]  (actors on this level)
```

**Map Z-layers**: `Map.TERRAIN=1`, `Map.TRAP=50`, `Map.ACTOR=100`, `Map.PROJECTILE=500`, `Map.OBJECT=1000`, `Map.TRIGGER=10000`

```lua
map(x, y, Map.TERRAIN)                       -- read terrain at (x,y)
map(x, y, Map.ACTOR, actor)                  -- place actor at (x,y)
map:checkAllEntities(x, y, "block_move", self)  -- query all entities at (x,y)
```

**Zone** manages: class setup (`zone:setup{npc_class, grid_class, ...}`), multiple levels, ego/affix system (`Zone.ego_rules`), and LRU level cache.

### Game Loop

**`GameEnergyBased`** (base): Each tick, all entities gain energy proportional to `energy.mod * global_speed`. Entities act when `energy.value >= energy_to_act` (default 1000, gained 100/tick).

**`GameTurnBased`** (inherits above): Adds pause mechanism — `game.paused = true` when waiting for player input, `false` after player acts. NPCs only advance while unpaused.

### Actor Interface Mixins (`engine/interface/`)

Actors are composed by inheriting mixins:
```lua
class.inherit(engine.Actor, interface.ActorTalents, interface.ActorStats, ...)
```

Key mixins:

**`ActorTalents`**:
- Define: `self:newTalentType{...}` / `self:newTalent{name, type, mode, cooldown, action, ...}` — auto-generates `T_TALENTNAME` constant
- Instance data: `self.talents = {T_FIREBALL=3}`, `self.talents_cd = {T_FIREBALL=5}`, `self.sustain_talents`
- Modes: `"activated"`, `"sustained"`, `"passive"`

**`ActorTemporaryEffects`** (Buff/Debuff):
- Define: `self:newEffect{name, activation, deactivation, on_timeout, ...}` — auto-generates `EFF_NAME` constant
- Instance: `self.tmp = {EFF_BURNING = {dur=5, power=10}}`
- Each turn: `:timedEffects()` counts down; calls `deactivation` when expired

**`ActorStats`**: `self:getStat("str")` returns final value (base + buffs); `self:addStat("str", 10)` for temporary bonuses.

**`ActorFOV`**: Calls C-layer `core.fov.calc_*`. Use `actor:computeFOV(range, "block_sight", callback)`, `actor:hasLOS(x,y)`, `actor:lineFOV(x,y)`.

**`ActorProject`** (projection/damage):
```lua
self:project({type="bolt", range=5}, tx, ty, DamageType.FIRE, 100, particles)
```
Internally resolves shape → iterates tiles with `lineFOV` → calls `DamageType.project(src, x, y, type, dam)` per hit tile.

Target shapes: `"bolt"`, `"beam"`, `"ball"` (with `radius`), `"cone"` (with `cone_angle`), `"hit"`.

**`ActorAI`**:
- `npc.ai = "dumb_talented"` selects the AI behavior
- Built-in AIs: `move_simple`, `move_dmap` (Dijkstra), `flee_simple`, `target_simple`, `dumb_talented`, `talented`
- `self:runAI(name, ...)` calls `ai_def[name](self, ...)` — AIs can compose each other

### Damage Type System (`engine/DamageType.lua`)

```lua
DamageType:newDamageType{
    name = "FIRE",
    projector = function(src, x, y, type, dam) ... end,
}
-- auto-generates DamageType.FIRE constant
```

Each damage type has its own projector function invoked by `ActorProject:project()`.

### Map Generation (`engine/generator/`)

All generators inherit `engine.Generator` and implement `:generate(lev, old_lev)`.

```lua
-- In zone definition:
generator = {
    map   = {class="engine.generator.map.Roomer", floor="FLOOR", wall="WALL", ...},
    actor = {class="engine.generator.actor.Random", nb_npc={10,15}, ...},
}
```

| Generator | Algorithm |
|-----------|-----------|
| `Rooms` / `RoomsLoader` | BSP split; RoomsLoader uses MST for room connectivity |
| `Cavern` | Cellular automaton smoothing |
| `Maze` | Recursive backtracking |
| `Forest` | Perlin noise for terrain distribution |
| `Heightmap` | Heightmap → terrain (mountains, plains, water) |
| `WaveFunctionCollapse` | Calls C++ WFC core (`src/wfc/`) |
| `Static` | Hand-crafted `.lua` map files |
| `GOL` | Game of Life cellular automaton |

Some generators use `engine/tilemaps/` as an intermediate char-code representation before mapping to actual entities.

### Save System (`engine/Savefile.lua`)

Each save = one zip file containing multiple Lua-serialized objects. Each object is stored separately by hash. On load, `__CLASSNAME` is used to restore metatables, then `:loaded()` is called. `__persistent_hooks` are automatically re-bound after load.

### UI Framework (`engine/ui/`)

Full UI widget library drawn in OpenGL via Lua. Base class: `ui/Base.lua`. Key widgets: `Button`, `Checkbox`, `Dialog`, `Dropdown`, `List`, `ListColumns`, `Tabs`, `Textbox`, `Textzone`, `TreeList`, `Slider`. Pre-built dialogs in `engine/dialogs/` cover inventory, store, quests, key-binding, video/audio options, etc.

---

## Game Modules (`game/modules/`)

Each module is a `.team` zip archive (or unpacked directory during development). Entry point is `init.lua`:

```lua
name = "My Game"
short_name = "mygame"
version = {1, 0, 0}
engine = {1, 7, 6, "te4"}
starter = "mod.load"   -- Lua path called on start
```

Modules inherit and override `engine.*` classes for game-specific rules. `example/` and `example_realtime/` are reference templates. `tome-1.7.6.team` is Tales of Maj'Eyal.

---

## Architecture Documentation

`architecture/overview.md` and `architecture/lua_engine_detail.md` contain detailed design documentation (written in Traditional Chinese).
