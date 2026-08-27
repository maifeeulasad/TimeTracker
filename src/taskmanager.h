/*
 * File: src/taskmanager.h
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>

class TaskManager : public QObject
{
    Q_OBJECT

public:
    enum State { Idle, Running, Paused };

    static TaskManager &instance();

    State     state()              const { return m_state; }
    int       currentTaskId()      const { return m_taskId; }
    QString   currentDescription() const { return m_desc; }
    int       elapsedSeconds()     const { return m_elapsed; }
    int       todayWorkSeconds()   const;

    QString formattedElapsed()    const;
    QString formattedTodayTotal() const;

public slots:
    bool startTask(const QString &description);
    bool stopTask();
    bool pauseTask();
    bool resumeTask();
    bool switchTask(const QString &newDescription);
    bool startBreak();
    bool endBreak();

signals:
    void stateChanged(State newState);
    void tick(int elapsedSeconds);
    void sleepDetected();
    void todayTotalChanged(int seconds);

private:
    TaskManager();
    ~TaskManager() override = default;
    TaskManager(const TaskManager &)            = delete;
    TaskManager &operator=(const TaskManager &) = delete;

    void onTick();
    void checkSleep();
    void refreshToday();

    State     m_state   = Idle;
    int       m_taskId  = -1;
    QString   m_desc;
    int       m_elapsed = 0;
    int       m_breakId = -1;

    QDateTime m_start;
    QDateTime m_lastTick;
    QTimer   *m_tickTimer  = nullptr;
    QTimer   *m_todayTimer = nullptr;

    int       m_todayWork  = 0;
};

#endif // TASKMANAGER_H
