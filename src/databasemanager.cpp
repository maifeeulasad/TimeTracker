/*
 * File: src/databasemanager.cpp
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#include "databasemanager.h"
#include "config.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <QDebug>

/* ================================================================== */
DatabaseManager::DatabaseManager() : QObject(nullptr) {}

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager inst;
    return inst;
}

/* ------------------------------------------------------------------ */
bool DatabaseManager::initialize()
{
    const QString path = Config::instance().databasePath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qCritical() << "DB open failed:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery p(m_db);
    p.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    p.exec(QStringLiteral("PRAGMA foreign_keys=ON"));

    return createTables();
}

bool DatabaseManager::isOpen() const { return m_db.isOpen(); }

/* ------------------------------------------------------------------ */
bool DatabaseManager::createTables()
{
    QSqlQuery q(m_db);

    if (!q.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS tasks ("
            "  id               INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  description      TEXT    NOT NULL,"
            "  started_at       TEXT    NOT NULL,"
            "  ended_at         TEXT,"
            "  duration_seconds INTEGER DEFAULT 0,"
            "  break_seconds    INTEGER DEFAULT 0,"
            "  status           TEXT    DEFAULT 'active'"
            "      CHECK(status IN ('active','paused','completed','cancelled')),"
            "  notes            TEXT,"
            "  created_at       TEXT DEFAULT (datetime('now','localtime')),"
            "  updated_at       TEXT DEFAULT (datetime('now','localtime'))"
            ")"))) {
        qCritical() << "tasks table:" << q.lastError().text();
        return false;
    }

    q.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_tasks_started ON tasks(started_at)"));
    q.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_tasks_status  ON tasks(status)"));

    if (!q.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS breaks ("
            "  id               INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  task_id          INTEGER NOT NULL,"
            "  started_at       TEXT    NOT NULL,"
            "  ended_at         TEXT,"
            "  duration_seconds INTEGER DEFAULT 0,"
            "  FOREIGN KEY (task_id) REFERENCES tasks(id) ON DELETE CASCADE"
            ")"))) {
        qCritical() << "breaks table:" << q.lastError().text();
        return false;
    }
    return true;
}

/* ------------------------------------------------------------------ */
TaskRecord DatabaseManager::readRecord(QSqlQuery &q) const
{
    TaskRecord r;
    r.id              = q.value(QStringLiteral("id")).toInt();
    r.description     = q.value(QStringLiteral("description")).toString();
    r.startedAt       = QDateTime::fromString(q.value(QStringLiteral("started_at")).toString(), Qt::ISODate);
    r.endedAt         = QDateTime::fromString(q.value(QStringLiteral("ended_at")).toString(),   Qt::ISODate);
    r.durationSeconds = q.value(QStringLiteral("duration_seconds")).toInt();
    r.breakSeconds    = q.value(QStringLiteral("break_seconds")).toInt();
    r.status          = q.value(QStringLiteral("status")).toString();
    r.notes           = q.value(QStringLiteral("notes")).toString();
    return r;
}

/* ---- CRUD -------------------------------------------------------- */
int DatabaseManager::createTask(const QString &description)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "INSERT INTO tasks (description, started_at, status) "
        "VALUES (?, datetime('now','localtime'), 'active')"));
    q.addBindValue(description);
    if (!q.exec()) { qCritical() << q.lastError().text(); return -1; }
    return q.lastInsertId().toInt();
}

bool DatabaseManager::updateTask(int id, const TaskRecord &r)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "UPDATE tasks SET description=?, started_at=?, ended_at=?, "
        "duration_seconds=?, break_seconds=?, status=?, notes=?, "
        "updated_at=datetime('now','localtime') WHERE id=?"));
    q.addBindValue(r.description);
    q.addBindValue(r.startedAt.toString(Qt::ISODate));
    q.addBindValue(r.endedAt.isValid() ? r.endedAt.toString(Qt::ISODate) : QVariant());
    q.addBindValue(r.durationSeconds);
    q.addBindValue(r.breakSeconds);
    q.addBindValue(r.status);
    q.addBindValue(r.notes);
    q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::completeTask(int id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "UPDATE tasks SET status='completed', ended_at=datetime('now','localtime'), "
        "duration_seconds = CAST((julianday(datetime('now','localtime')) - julianday(started_at))*86400 AS INTEGER) - break_seconds, "
        "updated_at=datetime('now','localtime') WHERE id=?"));
    q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::cancelTask(int id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "UPDATE tasks SET status='cancelled', ended_at=datetime('now','localtime'), "
        "updated_at=datetime('now','localtime') WHERE id=?"));
    q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::deleteTask(int id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM tasks WHERE id=?"));
    q.addBindValue(id);
    return q.exec();
}

TaskRecord DatabaseManager::getTask(int id) const
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT * FROM tasks WHERE id=?"));
    q.addBindValue(id);
    if (q.exec() && q.next()) return readRecord(q);
    return {};
}

TaskRecord DatabaseManager::getActiveTask() const
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT * FROM tasks WHERE status='active' ORDER BY started_at DESC LIMIT 1"));
    if (q.exec() && q.next()) return readRecord(q);
    return {};
}

/* ---- Breaks ------------------------------------------------------ */
int DatabaseManager::startBreak(int taskId)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "INSERT INTO breaks (task_id, started_at) VALUES (?, datetime('now','localtime'))"));
    q.addBindValue(taskId);
    if (!q.exec()) return -1;
    return q.lastInsertId().toInt();
}

bool DatabaseManager::endBreak(int breakId)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "UPDATE breaks SET ended_at=datetime('now','localtime'), "
        "duration_seconds=CAST((julianday(datetime('now','localtime'))-julianday(started_at))*86400 AS INTEGER) "
        "WHERE id=?"));
    q.addBindValue(breakId);
    if (!q.exec()) return false;

    /* roll up break total into the parent task */
    q.prepare(QStringLiteral(
        "UPDATE tasks SET break_seconds = "
        "  (SELECT COALESCE(SUM(duration_seconds),0) FROM breaks WHERE task_id=tasks.id), "
        "  updated_at=datetime('now','localtime') "
        "WHERE id=(SELECT task_id FROM breaks WHERE id=?)"));
    q.addBindValue(breakId);
    return q.exec();
}

/* ---- Queries ----------------------------------------------------- */
QList<TaskRecord> DatabaseManager::getTasksByDateRange(const QDate &from, const QDate &to) const
{
    QList<TaskRecord> list;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT * FROM tasks WHERE date(started_at) BETWEEN ? AND ? ORDER BY started_at DESC"));
    q.addBindValue(from.toString(Qt::ISODate));
    q.addBindValue(to.toString(Qt::ISODate));
    if (q.exec()) while (q.next()) list.append(readRecord(q));
    return list;
}

QList<TaskRecord> DatabaseManager::getTasksByDescription(const QString &needle) const
{
    QList<TaskRecord> list;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT * FROM tasks WHERE description LIKE ? ORDER BY started_at DESC"));
    q.addBindValue(QLatin1Char('%') + needle + QLatin1Char('%'));
    if (q.exec()) while (q.next()) list.append(readRecord(q));
    return list;
}

QList<TaskRecord> DatabaseManager::getAllTasks(int limit, int offset) const
{
    QList<TaskRecord> list;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT * FROM tasks ORDER BY started_at DESC LIMIT ? OFFSET ?"));
    q.addBindValue(limit);
    q.addBindValue(offset);
    if (q.exec()) while (q.next()) list.append(readRecord(q));
    return list;
}

DailySummary DatabaseManager::getDailySummary(const QDate &date) const
{
    DailySummary s; s.date = date;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT COALESCE(SUM(duration_seconds),0) AS tw,"
        "       COALESCE(SUM(break_seconds),0)    AS tb,"
        "       COUNT(*)                          AS tc,"
        "       SUM(CASE WHEN status='completed' THEN 1 ELSE 0 END) AS cc "
        "FROM tasks WHERE date(started_at)=? AND status<>'cancelled'"));
    q.addBindValue(date.toString(Qt::ISODate));
    if (q.exec() && q.next()) {
        s.totalWorkSeconds  = q.value(0).toInt();
        s.totalBreakSeconds = q.value(1).toInt();
        s.taskCount         = q.value(2).toInt();
        s.completedCount    = q.value(3).toInt();
    }
    return s;
}

QList<DailySummary> DatabaseManager::getDailySummaries(const QDate &from, const QDate &to) const
{
    QList<DailySummary> list;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT date(started_at) AS d,"
        "       COALESCE(SUM(duration_seconds),0),"
        "       COALESCE(SUM(break_seconds),0),"
        "       COUNT(*),"
        "       SUM(CASE WHEN status='completed' THEN 1 ELSE 0 END) "
        "FROM tasks WHERE date(started_at) BETWEEN ? AND ? AND status<>'cancelled' "
        "GROUP BY d ORDER BY d"));
    q.addBindValue(from.toString(Qt::ISODate));
    q.addBindValue(to.toString(Qt::ISODate));
    if (q.exec()) {
        while (q.next()) {
            DailySummary s;
            s.date              = QDate::fromString(q.value(0).toString(), Qt::ISODate);
            s.totalWorkSeconds  = q.value(1).toInt();
            s.totalBreakSeconds = q.value(2).toInt();
            s.taskCount         = q.value(3).toInt();
            s.completedCount    = q.value(4).toInt();
            list.append(s);
        }
    }
    return list;
}

/* ---- Export ------------------------------------------------------ */
bool DatabaseManager::exportToCsv(const QString &path, const QDate &from, const QDate &to) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "ID,Description,Started,Ended,Duration_sec,Break_sec,Status,Notes\n";

    for (const auto &t : getTasksByDateRange(from, to)) {
        QString desc  = t.description;  desc.replace(QLatin1Char('"'), QStringLiteral("\"\""));
        QString notes = t.notes;        notes.replace(QLatin1Char('"'), QStringLiteral("\"\""));
        out << t.id << ','
            << '"' << desc  << "\","
            << t.startedAt.toString(Qt::ISODate) << ','
            << (t.endedAt.isValid() ? t.endedAt.toString(Qt::ISODate) : QString()) << ','
            << t.durationSeconds << ','
            << t.breakSeconds    << ','
            << t.status          << ','
            << '"' << notes << "\"\n";
    }
    return true;
}

/* ---- Info -------------------------------------------------------- */
QString DatabaseManager::databasePath() const { return m_db.databaseName(); }

int DatabaseManager::totalTaskCount() const
{
    QSqlQuery q(m_db);
    q.exec(QStringLiteral("SELECT COUNT(*) FROM tasks"));
    return q.next() ? q.value(0).toInt() : 0;
}
