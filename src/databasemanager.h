/*
 * File: src/databasemanager.h
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QDateTime>
#include <QList>

/* ---- data structures ---- */
struct TaskRecord {
    int       id              = 0;
    QString   description;
    QDateTime startedAt;
    QDateTime endedAt;
    int       durationSeconds = 0;
    int       breakSeconds    = 0;
    QString   status;           // active | paused | completed | cancelled
    QString   notes;
};

struct DailySummary {
    QDate date;
    int   totalWorkSeconds  = 0;
    int   totalBreakSeconds = 0;
    int   taskCount         = 0;
    int   completedCount    = 0;
};

/* ---- singleton ---- */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager &instance();

    bool initialize();
    bool isOpen() const;

    /* CRUD */
    int  createTask(const QString &description);
    bool updateTask(int id, const TaskRecord &rec);
    bool completeTask(int id);
    bool cancelTask(int id);
    bool deleteTask(int id);
    TaskRecord getTask(int id) const;
    TaskRecord getActiveTask() const;

    /* Breaks */
    int  startBreak(int taskId);
    bool endBreak(int breakId);

    /* Queries */
    QList<TaskRecord>   getTasksByDateRange(const QDate &from, const QDate &to) const;
    QList<TaskRecord>   getTasksByDescription(const QString &needle) const;
    QList<TaskRecord>   getAllTasks(int limit = 200, int offset = 0) const;
    DailySummary        getDailySummary(const QDate &date) const;
    QList<DailySummary> getDailySummaries(const QDate &from, const QDate &to) const;

    /* Export */
    bool exportToCsv(const QString &path, const QDate &from, const QDate &to) const;

    /* Info */
    QString databasePath() const;
    int     totalTaskCount() const;

private:
    DatabaseManager();
    ~DatabaseManager() override = default;
    DatabaseManager(const DatabaseManager &)            = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    bool       createTables();
    TaskRecord readRecord(class QSqlQuery &q) const;

    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
