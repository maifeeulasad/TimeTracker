# Contributing to TimeTracker

Thanks for your interest in improving TimeTracker.

## Getting started

```bash
sudo apt install build-essential qtbase5-dev qtbase5-dev-tools libqt5sql5-sqlite pkg-config
git clone https://github.com/maifeeulasad/TimeTracker.git
cd TimeTracker
make            # or: make QT_VERSION=6
./timetracker
```

## Development guidelines

- **Language / standard:** C++17, Qt Widgets.
- **Style:** match the surrounding code — 4-space indent, `m_` prefix for
  members, `QStringLiteral` for literals, one class per file pair.
- Keep the managers (`Config`, `DatabaseManager`, `TaskManager`,
  `NotificationManager`) as singletons; UI classes own no persistent state.
- Every new `.h` with `Q_OBJECT` is picked up automatically by the `Makefile`
  moc rule — no manual wiring needed.
- Prefer prepared statements for all SQL.

## Commit messages

- Use short, imperative subject lines (`Add ...`, `Fix ...`, `Refactor ...`).
- Group related changes into one commit; keep unrelated changes separate.
- If a change was produced with AI assistance, keep the
  `Co-Authored-By:` trailer (see [AUTHORS.md](AUTHORS.md)).

## Pull requests

1. Fork and create a feature branch.
2. Ensure `make` and `make QT_VERSION=6` both build cleanly with no new warnings.
3. Update `CHANGELOG.md` under an "Unreleased" heading.
4. Open the PR against `main` with a clear description and screenshots for UI
   changes.

## Reporting bugs

Open an issue with your OS, Qt version (`qmake --version`), reproduction steps,
and any console output.
