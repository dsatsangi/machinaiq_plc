# SKILL.md — MachinaIQ UPLC

## Architecture Overview

```
┌─────────────────────────────────────────────┐
│                  UI Layer                   │
│  MainWindow → HomeScreen / SettingsScreen   │
│  Widgets: StatusBanner, ScrewCount,         │
│           ChannelSelector, StatBox          │
└──────────────┬──────────────────────────────┘
               │ Qt signals/slots (main thread)
┌──────────────▼──────────────────────────────┐
│              State Manager                  │
│  Single source of truth (AppState struct)   │
│  Thread-safe writes; emits on main thread   │
└──────────────┬──────────────────────────────┘
               │ reads/writes
┌──────────────▼──────────────────────────────┐
│           Communication Layer               │
│  CommManager (main thread coordinator)      │
│  SerialHandler  ←→  QThread (serial)        │
│  TcpHandler     ←→  QThread (tcp)           │
└─────────────────────────────────────────────┘
               │ load/save
┌──────────────▼──────────────────────────────┐
│            Config Manager                   │
│  JSON persistence, hot-reload capable       │
│  Hierarchy: Station→SD→Pattern→Channel      │
└─────────────────────────────────────────────┘
```

## Coding Standards

- C++17. No exceptions in production paths — use return values.
- All cross-thread signals use `Qt::QueuedConnection` (explicit where ambiguous).
- No raw `new` in UI constructors except Qt parent-owned widgets.
- `Q_DISABLE_COPY_MOVE` on all singletons.
- Singleton pattern: `static T inst; return inst;` — no dynamic allocation.
- Public API of managers is minimal: expose only what callers need.
- Zero `qDebug` in release builds: wrap in `#ifndef NDEBUG` or use `qCDebug`.

## Naming Conventions

| Item | Convention | Example |
|------|-----------|---------|
| Classes | PascalCase | `StateManager` |
| Member vars | `m_camelCase` | `m_statusLabel` |
| Signals | `verbNoun` past tense where possible | `cycleUpdated` |
| Slots | `onEvent` | `onCycleUpdated` |
| QSS object names | `camelCase` string | `"dataCard"` |
| Files | PascalCase for class files | `HomeScreen.cpp` |

## Threading Model

```
Main Thread (Qt event loop)
  ├─ All UI widgets
  ├─ StateManager (emits queued → main)
  ├─ CommManager (routes messages, sends commands)
  └─ ConfigManager

Serial Thread  (QThread)
  └─ SerialHandler (owns QSerialPort)

TCP Thread  (QThread)
  └─ TcpHandler (owns QTcpSocket)
```

- IO workers live in their own `QThread`. They must never touch UI.
- State writes from workers go through `StateManager` setters (mutex-protected).
- `StateManager` always emits signals on the main thread via `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`.
- UI slots connect directly to `StateManager` signals — no extra `Qt::QueuedConnection` needed.

## Communication Protocol

Newline-delimited ASCII frames. Same format for Serial and TCP.

### Inbound (device → HMI)

| Frame | Meaning |
|-------|---------|
| `SCREW:<cur>:<tot>:<ms>` | Screw count update + cycle time |
| `STATUS:<OK\|NOK\|RUNNING\|IDLE>` | Cycle pass/fail/running |
| `COUNTER:<total>:<pass>:<reject>` | Session counters |
| `CHANNEL:<n>` | Active channel changed (1–8) |
| `PROGRAM:<id>` | Active program name |
| `REVERSE:<ON\|OFF>` | Reverse state |
| `MODE:<AUTO\|MANUAL>` | Drive mode |
| `SYS:<ONLINE\|OFFLINE>` | System connectivity |

### Outbound (HMI → device)

| Frame | Action |
|-------|--------|
| `SET_CHANNEL:<n>` | Select channel |
| `REVERSE:<ENABLE\|DISABLE>` | Toggle reverse |
| `MODE:<AUTO\|MANUAL>` | Set drive mode |
| `RESET` | Reset cycle/counters |

## Configuration Hierarchy Limits

| Level | Max Items |
|-------|-----------|
| Station | 1 (root) |
| Screwdrivers per Station | 5 |
| Patterns per Screwdriver | 5 |
| Channels per Pattern | 8 |

## Extension Guidelines

### Adding a new inbound command
1. Add a `else if (cmd == "NEW_CMD" ...)` branch in `CommManager::parseMessage()`.
2. Add the corresponding setter in `StateManager` + signal.
3. Connect the new signal in the appropriate widget's `connectState()`.

### Adding a new UI screen
1. Create `src/ui/NewScreen.h/.cpp` inheriting `QWidget`.
2. Add `newScreenRequested()` signal to the preceding screen.
3. Wire navigation in `MainWindow` via the stacked widget.
4. Do not put business logic in screens — keep them as thin signal translators.

### Adding a new config field
1. Add the field to the relevant struct in `HierarchyModels.h`.
2. Update `ConfigManager::stationFromJson` and `stationToJson`.
3. Bump the config schema version in `default_config.json` if breaking.

## Anti-Patterns (do not do these)

- **No direct QSerialPort/QTcpSocket calls from UI** — always go through CommManager.
- **No blocking calls on the main thread** — no `QThread::sleep`, no synchronous socket reads.
- **No setState from UI** — UI calls CommManager; the device echoes back; CommManager updates StateManager. The state is always device-authoritative.
- **No singleton construction before QApplication** — all `instance()` calls happen in `Application::run()`.
- **No `qApp->processEvents()`** — fix the non-blocking design instead.
- **No hardcoded port/host in source** — always from CLI args or config file.

## Pi Zero Performance Rules

- Do not use `QGraphicsView`/`QGraphicsScene` — too heavy.
- Prefer `QFrame` + `QLabel` over custom `paintEvent` where possible.
- Avoid drop-shadows and heavy CSS gradients; flat colours render fast on fbdev.
- Profile with `perf` on-device before adding visual polish.
- Keep the stylesheet static (loaded once at startup, no runtime changes).
