/*
 * File: src/notificationmanager.cpp
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#include "notificationmanager.h"
#include "config.h"
#include "taskmanager.h"

#include <QApplication>
#include <QStyle>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

/* ================================================================== */
NotificationManager::NotificationManager()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &NotificationManager::fireNotification);

    m_tray = new QSystemTrayIcon(this);
    m_tray->setIcon(QIcon::fromTheme(
        QStringLiteral("preferences-system-time"),
        QApplication::style()->standardIcon(QStyle::SP_ComputerIcon)));
    m_tray->setVisible(true);

    connect(&Config::instance(), &Config::changed,
            this, &NotificationManager::onConfigChanged);
}

NotificationManager &NotificationManager::instance()
{
    static NotificationManager inst;
    return inst;
}

void NotificationManager::setMainWindow(QWidget *w) { m_mainWin = w; }

void NotificationManager::start()
{
    if (Config::instance().notificationsEnabled()
        && TaskManager::instance().state() == TaskManager::Running) {
        m_timer->start(Config::instance().notificationIntervalMinutes() * 60'000);
    }
}

void NotificationManager::stop()  { m_timer->stop(); }

void NotificationManager::reset()
{
    if (m_timer->isActive())
        m_timer->start(Config::instance().notificationIntervalMinutes() * 60'000);
}

void NotificationManager::onConfigChanged()
{
    if (Config::instance().notificationsEnabled()
        && TaskManager::instance().state() == TaskManager::Running)
        start();
    else
        stop();
}

/* ------------------------------------------------------------------ */
void NotificationManager::fireNotification()
{
    if (TaskManager::instance().state() != TaskManager::Running) return;
    if (m_activeDlg) return;                       // previous dialog still open

    emit notificationFired();

    if (Config::instance().flashOnNotification() && m_mainWin)
        QApplication::alert(m_mainWin, 3000);

    const QString task    = TaskManager::instance().currentDescription();
    const QString elapsed = TaskManager::instance().formattedElapsed();
    m_tray->showMessage(
        QStringLiteral("TimeTracker Check-In"),
        QStringLiteral("Working on:\n\"%1\"\nfor %2\nStill on it?").arg(task, elapsed),
        QSystemTrayIcon::Information, 10000);

    showCheckDialog();
}

void NotificationManager::showCheckDialog()
{
    auto *dlg = new QDialog(m_mainWin);
    dlg->setWindowTitle(QStringLiteral("TimeTracker — Task Check-In"));
    dlg->setWindowFlags(dlg->windowFlags() | Qt::WindowStaysOnTopHint);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setMinimumWidth(400);

    dlg->setStyleSheet(QStringLiteral(
        "QDialog      { background:#1a1a2e; }"
        "QLabel       { color:#e8e4df; }"
        "QPushButton  { padding:8px 16px; border-radius:4px; font-weight:bold; }"));

    auto *lay = new QVBoxLayout(dlg);
    lay->setSpacing(12);
    lay->setContentsMargins(20, 20, 20, 20);

    auto *title = new QLabel(QStringLiteral("Time Check-In"));
    title->setStyleSheet(QStringLiteral("font-size:16px; font-weight:bold; color:#ff6b35;"));
    lay->addWidget(title);

    const QString task    = TaskManager::instance().currentDescription();
    const QString elapsed = TaskManager::instance().formattedElapsed();
    auto *msg = new QLabel(QStringLiteral(
        "You've been working on:\n\"%1\"\nfor %2\n\nAre you still on this task?")
        .arg(task, elapsed));
    msg->setWordWrap(true);
    lay->addWidget(msg);

    auto *btns = new QHBoxLayout; btns->setSpacing(8);

    auto *contBtn  = new QPushButton(QStringLiteral("Continue"));
    contBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:#2d7d46;color:#fff;}QPushButton:hover{background:#3a9d56;}"));

    auto *swBtn    = new QPushButton(QStringLiteral("Switch Task"));
    swBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:#2a5298;color:#fff;}QPushButton:hover{background:#3a6ab8;}"));

    auto *brkBtn   = new QPushButton(QStringLiteral("Take Break"));
    brkBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:#8b6914;color:#fff;}QPushButton:hover{background:#ab8924;}"));

    btns->addWidget(contBtn);
    btns->addWidget(swBtn);
    btns->addWidget(brkBtn);
    lay->addLayout(btns);

    m_activeDlg = dlg;

    connect(contBtn, &QPushButton::clicked, [this, dlg]() {
        emit userContinued();
        reset();
        m_activeDlg = nullptr;
        dlg->accept();
    });
    connect(swBtn, &QPushButton::clicked, [this, dlg]() {
        emit userSwitchTask();
        m_activeDlg = nullptr;
        dlg->accept();
    });
    connect(brkBtn, &QPushButton::clicked, [this, dlg]() {
        emit userTakeBreak();
        m_activeDlg = nullptr;
        dlg->accept();
    });

    /* auto-dismiss */
    const int ad = Config::instance().autoDismissSeconds();
    if (ad > 0) {
        auto *dt = new QTimer(dlg); dt->setSingleShot(true);
        connect(dt, &QTimer::timeout, [this, dlg]() {
            emit userContinued();
            reset();
            m_activeDlg = nullptr;
            dlg->accept();
        });
        dt->start(ad * 1000);
    }

    dlg->show();
}

void NotificationManager::showNotification(const QString &title, const QString &body)
{
    m_tray->showMessage(title, body, QSystemTrayIcon::Information, 5000);
}
