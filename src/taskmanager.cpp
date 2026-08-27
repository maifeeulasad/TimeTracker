/*
 * File: src/taskmanager.cpp
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#include "taskmanager.h"
#include "databasemanager.h"
#include "config.h"

/* ================================================================== */
TaskManager::TaskManager()
{
    m_tickTimer = new QTimer(this);
    m_tickTimer->setInterval(1000);
    connect(m_tickTimer, &QTimer::timeout, this, &TaskManager::onTick);

    m_todayTimer = new QTimer(this);
    m_todayTimer->setInterval(60'000);
    connect(m_todayTimer, &QTimer::timeout, this, &TaskManager::refreshToday);
    m_todayTimer->start();

    refreshToday();

    /* Resume an active task from a previous session */
    const auto active = DatabaseManager::instance().getActiveTask();
    if (active.id > 0 && active.status == QLatin1String("active")) {
        m_taskId  = active.id;
        m_desc    = active.description;
        m_start   = active.startedAt;
        m_elapsed = static_cast<int>(active.startedAt.secsTo(QDateTime::currentDateTime()));
        m_state   = Running;
        m_lastTick = QDateTime::currentDateTime();
        m_tickTimer->start();
        emit stateChanged(m_state);
    }
}

TaskManager &TaskManager::instance()
{
    static TaskManager inst;
    return inst;
}

/* ---- public slots ------------------------------------------------ */
bool TaskManager::startTask(const QString &description)
{
    if (description.trimmed().isEmpty()) return false;
    if (m_state != Idle) stopTask();

    const int id = DatabaseManager::instance().createTask(description);
    if (id < 0) return false;

    m_taskId  = id;
    m_desc    = description;
    m_elapsed = 0;
    m_start   = QDateTime::currentDateTime();
    m_lastTick = m_start;
    m_state   = Running;
    m_tickTimer->start();

    emit stateChanged(m_state);
    refreshToday();
    return true;
}

bool TaskManager::stopTask()
{
    if (m_state == Idle) return false;
    if (m_breakId > 0) endBreak();

    DatabaseManager::instance().completeTask(m_taskId);

    m_state   = Idle;
    m_taskId  = -1;
    m_desc.clear();
    m_elapsed = 0;
    m_tickTimer->stop();

    emit stateChanged(m_state);
    refreshToday();
    return true;
}

bool TaskManager::pauseTask()
{
    if (m_state != Running) return false;
    m_state = Paused;
    m_tickTimer->stop();
    emit stateChanged(m_state);
    return true;
}

bool TaskManager::resumeTask()
{
    if (m_state != Paused) return false;
    m_state    = Running;
    m_lastTick = QDateTime::currentDateTime();
    m_tickTimer->start();
    emit stateChanged(m_state);
    return true;
}

bool TaskManager::switchTask(const QString &newDescription)
{
    if (newDescription.trimmed().isEmpty()) return false;
    stopTask();
    return startTask(newDescription);
}

bool TaskManager::startBreak()
{
    if (m_state != Running || m_taskId < 0) return false;
    m_breakId = DatabaseManager::instance().startBreak(m_taskId);
    if (m_breakId < 0) return false;
    return pauseTask();
}

bool TaskManager::endBreak()
{
    if (m_breakId < 0) return false;
    DatabaseManager::instance().endBreak(m_breakId);
    m_breakId = -1;
    return resumeTask();
}

/* ---- private ----------------------------------------------------- */
void TaskManager::onTick()
{
    checkSleep();
    if (m_state == Running) {
        m_elapsed = static_cast<int>(m_start.secsTo(QDateTime::currentDateTime()));
        emit tick(m_elapsed);
    }
}

void TaskManager::checkSleep()
{
    const QDateTime now  = QDateTime::currentDateTime();
    const qint64    gap  = m_lastTick.secsTo(now);

    if (gap > Config::instance().sleepThresholdSeconds()) {
        if (Config::instance().autoPauseOnSleep()) {
            pauseTask();
            emit sleepDetected();
        }
    }
    m_lastTick = now;
}

void TaskManager::refreshToday()
{
    auto s = DatabaseManager::instance().getDailySummary(QDate::currentDate());
    m_todayWork = s.totalWorkSeconds;
    if (m_state == Running) m_todayWork += m_elapsed;
    emit todayTotalChanged(m_todayWork);
}

int TaskManager::todayWorkSeconds() const { return m_todayWork; }

QString TaskManager::formattedElapsed() const
{
    return QStringLiteral("%1:%2:%3")
        .arg(m_elapsed / 3600, 2, 10, QLatin1Char('0'))
        .arg((m_elapsed % 3600) / 60, 2, 10, QLatin1Char('0'))
        .arg(m_elapsed % 60, 2, 10, QLatin1Char('0'));
}

QString TaskManager::formattedTodayTotal() const
{
    return QStringLiteral("%1h %2m")
        .arg(m_todayWork / 3600)
        .arg((m_todayWork % 3600) / 60, 2, 10, QLatin1Char('0'));
}
