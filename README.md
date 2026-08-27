# TimeTracker

A persistent, always-on-top time-tracking application for Ubuntu desktops.

[![build](https://github.com/maifeeulasad/TimeTracker/actions/workflows/build.yml/badge.svg)](https://github.com/maifeeulasad/TimeTracker/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Qt 5 / 6](https://img.shields.io/badge/Qt-5%20%7C%206-41cd52.svg)

**Author:** Maifee Ul Asad · **License:** MIT

---

## Features

- **Always-on-top** compact widget — never lose sight of what you're doing
- **Periodic check-ins** (configurable, default every 5 minutes) — a popup asks
  "Still working on this?" so you stay focused
- **Sleep / wake detection** — timer auto-pauses when your machine suspends
- **SQLite storage** — all data in a single portable `.db` file
- **Range queries & CSV export** — filter history by date range and task name
- **System tray** integration — minimise to tray, restore with one click
- **Auto-start on login** — optional `.desktop` file in `~/.config/autostart`
- **Everything configurable** — notifications, opacity, sleep threshold, database path, …
- **Docker GUI support** — run inside a container with X11 / Wayland forwarding

---

## Prerequisites

### Ubuntu / Debian

```bash
sudo apt install build-essential qtbase5-dev qtbase5-dev-tools libqt5sql5-sqlite pkg-config
```

For Qt 6:

```bash
sudo apt install qt6-base-dev qt6-base-dev-tools libqt6sql6-sqlite pkg-config
```

---

## Build

```bash
# Qt 5 (default)
make

# Qt 6
make QT_VERSION=6

# Install system-wide
sudo make install
```

---

## Docker

```bash
# Allow Docker to access your display
xhost +local:docker

# Build & run
docker compose up --build

# Or manually
docker build -t timetracker .
docker run --rm -it \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v tt-data:/root/.local/share/TimeTracker \
  timetracker
```

See the `Dockerfile` comments for Wayland instructions.

---

## Usage

1. Launch `timetracker` (or find it in your app menu after `make install`).
2. Type a task description and press **Enter** or click **Start**.
3. Every N minutes (default 5, configurable) a check-in dialog appears.
4. Click **Continue**, **Switch Task**, or **Take Break**.
5. Click **Stop** when you're done — the task is saved as *completed*.
6. Use **History** to browse, filter, and export your data.

### Keyboard

| Key     | Action                              |
|---------|-------------------------------------|
| Enter   | Start task / switch to new task     |
| (input) | Type task description               |

---

## Configuration

Open **Settings** from the main window. Everything is persisted in:

```
~/.config/MaifeeUlAsad/TimeTracker/timetracker.conf
```

| Setting              | Default | Range     |
|----------------------|---------|-----------|
| Notification interval| 5 min   | 1–120 min |
| Auto-dismiss         | 30 sec  | 0–300 sec |
| Always on top        | yes     |           |
| Window opacity       | 0.95    | 0.30–1.00 |
| Auto-start on login  | yes     |           |
| Start minimised      | no      |           |
| Auto-pause on sleep  | yes     |           |
| Sleep threshold      | 10 sec  | 5–120 sec |

---

## Database & External Access

The SQLite database lives at:

```
~/.local/share/TimeTracker/timetracker.db
```

(Change this in Settings → Database → Browse.)

### Schema

```sql
-- Tasks
CREATE TABLE tasks (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    description      TEXT NOT NULL,
    started_at       TEXT NOT NULL,       -- ISO-8601
    ended_at         TEXT,
    duration_seconds INTEGER DEFAULT 0,
    break_seconds    INTEGER DEFAULT 0,
    status           TEXT DEFAULT 'active', -- active|paused|completed|cancelled
    notes            TEXT,
    created_at       TEXT,
    updated_at       TEXT
);

-- Breaks (per-task)
CREATE TABLE breaks (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    task_id          INTEGER NOT NULL REFERENCES tasks(id) ON DELETE CASCADE,
    started_at       TEXT NOT NULL,
    ended_at         TEXT,
    duration_seconds INTEGER DEFAULT 0
);
```

### Example Queries

```sql
-- All tasks from last 7 days
SELECT * FROM tasks
WHERE date(started_at) >= date('now', '-7 days')
ORDER BY started_at DESC;

-- Daily work totals for a date range
SELECT date(started_at) AS day,
       SUM(duration_seconds) AS work_sec,
       SUM(break_seconds)    AS break_sec,
       COUNT(*)              AS tasks
FROM tasks
WHERE date(started_at) BETWEEN '2026-01-01' AND '2026-01-31'
  AND status <> 'cancelled'
GROUP BY day ORDER BY day;

-- Search tasks by keyword
SELECT * FROM tasks
WHERE description LIKE '%refactor%'
ORDER BY started_at DESC;

-- Average task duration
SELECT AVG(duration_seconds) FROM tasks WHERE status = 'completed';
```

You can use any SQLite client: `sqlite3`, DB Browser for SQLite, DBeaver, etc.

---

## Project Structure

```
TimeTracker/
├── src/
│   ├── main.cpp                  — entry point, single-instance guard
│   ├── config.h / .cpp           — all settings (QSettings)
│   ├── databasemanager.h / .cpp  — SQLite CRUD, range queries, CSV export
│   ├── taskmanager.h / .cpp      — task lifecycle, sleep detection
│   ├── notificationmanager.h/.cpp— periodic check-in dialogs
│   ├── mainwindow.h / .cpp       — compact always-on-top UI + tray
│   ├── settingsdialog.h / .cpp   — full settings editor
│   └── historydialog.h / .cpp    — history browser with date filter
├── .github/workflows/build.yml   — CI: builds against Qt 5 and Qt 6
├── Makefile                      — builds with g++ + pkg-config + moc
├── Dockerfile                    — Ubuntu 22.04 + Qt 5
├── docker-compose.yml            — X11 forwarding, named volumes
├── timetracker.desktop           — freedesktop launcher
├── AUTHORS.md / CHANGELOG.md / CONTRIBUTING.md
└── README.md
```

---

## Credits

Designed by **Maifee Ul Asad**. The initial implementation was generated with
**[Xiaomi MiMo](https://github.com/XiaomiMiMo)** from that design and then
reviewed and integrated by the author. See [AUTHORS.md](AUTHORS.md) for details.

## License

MIT — see [LICENSE](LICENSE) and the per-file headers.
