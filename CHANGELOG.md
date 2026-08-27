# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed

- Docker/compose/README now mount the data volume at the real Qt data path
  (`~/.local/share/MaifeeUlAsad/TimeTracker`), so the SQLite database actually
  persists across container runs.

## [1.0.0] - 2026-08-27

### Added

- Compact always-on-top time-tracking widget with system-tray integration.
- Task lifecycle: start, stop, pause/resume, switch, and per-task breaks.
- Periodic check-in dialog ("Still working on this?") with Continue / Switch /
  Take Break actions and configurable auto-dismiss.
- Sleep / wake detection that auto-pauses the running timer.
- SQLite storage (`tasks`, `breaks`) with WAL mode and indexes.
- History browser with date-range and task-name filtering, plus CSV export.
- Settings dialog covering notifications, window, startup, sleep, and database
  path, persisted via `QSettings`.
- Optional auto-start on login through a generated `~/.config/autostart` entry.
- Single-instance guard via `QSharedMemory`.
- Build tooling: `Makefile` (Qt 5 / Qt 6), `Dockerfile`, `docker-compose.yml`,
  freedesktop `.desktop` launcher, and a GitHub Actions build workflow.

[1.0.0]: https://github.com/maifeeulasad/TimeTracker/releases/tag/v1.0.0
