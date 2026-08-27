/*
 * File: src/mainwindow.cpp
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#include "mainwindow.h"
#include "taskmanager.h"
#include "notificationmanager.h"
#include "settingsdialog.h"
#include "historydialog.h"
#include "config.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QStyle>
#include <QCloseEvent>

/* ================================================================== */
MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    buildUi();
    buildTray();
    connectSignals();
    applyStyle();
    refreshWindowFlags();

    setWindowTitle(QStringLiteral("TimeTracker"));
    setMinimumWidth(500);
    resize(540, 145);
}

/* ---- UI ---------------------------------------------------------- */
void MainWindow::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setSpacing(6);
    root->setContentsMargins(10, 10, 10, 10);

    /* Row 1 — input + buttons */
    auto *r1 = new QHBoxLayout; r1->setSpacing(6);
    m_input    = new QLineEdit;  m_input->setPlaceholderText(QStringLiteral("What are you working on?"));
    m_input->setObjectName(QStringLiteral("taskInput"));
    m_startBtn = new QPushButton(QStringLiteral("Start"));
    m_startBtn->setObjectName(QStringLiteral("startBtn")); m_startBtn->setFixedWidth(72);
    m_breakBtn = new QPushButton(QStringLiteral("Break"));
    m_breakBtn->setObjectName(QStringLiteral("breakBtn")); m_breakBtn->setFixedWidth(64);
    m_breakBtn->setEnabled(false);
    r1->addWidget(m_input, 1);
    r1->addWidget(m_startBtn);
    r1->addWidget(m_breakBtn);
    root->addLayout(r1);

    /* Row 2 — status / timer / today */
    auto *r2 = new QHBoxLayout; r2->setSpacing(10);
    m_statusLbl = new QLabel(QStringLiteral("Idle"));
    m_statusLbl->setObjectName(QStringLiteral("statusLbl"));
    m_timerLbl  = new QLabel(QStringLiteral("00:00:00"));
    m_timerLbl->setObjectName(QStringLiteral("timerLbl"));
    m_todayLbl  = new QLabel(QStringLiteral("Today: 0h 00m"));
    m_todayLbl->setObjectName(QStringLiteral("todayLbl"));
    r2->addWidget(m_statusLbl, 1);
    r2->addWidget(m_timerLbl);
    r2->addSpacing(15);
    r2->addWidget(m_todayLbl);
    root->addLayout(r2);

    /* Row 3 — bottom buttons */
    auto *r3 = new QHBoxLayout;
    m_histBtn = new QPushButton(QStringLiteral("History"));
    m_histBtn->setObjectName(QStringLiteral("flatBtn"));
    m_settBtn = new QPushButton(QStringLiteral("Settings"));
    m_settBtn->setObjectName(QStringLiteral("flatBtn"));
    m_minBtn  = new QPushButton(QStringLiteral("Minimize"));
    m_minBtn->setObjectName(QStringLiteral("flatBtn"));
    r3->addStretch();
    r3->addWidget(m_histBtn);
    r3->addWidget(m_settBtn);
    r3->addWidget(m_minBtn);
    root->addLayout(r3);
}

void MainWindow::buildTray()
{
    m_tray = new QSystemTrayIcon(this);
    m_tray->setIcon(QIcon::fromTheme(
        QStringLiteral("preferences-system-time"),
        QApplication::style()->standardIcon(QStyle::SP_ComputerIcon)));

    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction(QStringLiteral("Show"),   this, &QWidget::showNormal);
    m_trayMenu->addAction(QStringLiteral("History"), this, &MainWindow::showHistory);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(QStringLiteral("Quit"),    qApp, &QApplication::quit);
    m_tray->setContextMenu(m_trayMenu);
    m_tray->setVisible(true);

    connect(m_tray, &QSystemTrayIcon::activated, this, &MainWindow::trayActivated);
}

void MainWindow::connectSignals()
{
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartStop);
    connect(m_breakBtn, &QPushButton::clicked, this, &MainWindow::onBreak);
    connect(m_input,    &QLineEdit::returnPressed, this, &MainWindow::onInputReturn);
    connect(m_histBtn,  &QPushButton::clicked, this, &MainWindow::showHistory);
    connect(m_settBtn,  &QPushButton::clicked, this, &MainWindow::showSettings);
    connect(m_minBtn,   &QPushButton::clicked, this, &MainWindow::toTray);

    auto &tm = TaskManager::instance();
    connect(&tm, &TaskManager::tick,          this, &MainWindow::onTick);
    connect(&tm, &TaskManager::stateChanged,  this, &MainWindow::onStateChanged);
    connect(&tm, &TaskManager::todayTotalChanged, this, &MainWindow::onTodayChanged);
    connect(&tm, &TaskManager::sleepDetected, this, &MainWindow::onSleep);

    auto &nm = NotificationManager::instance();
    nm.setMainWindow(this);
    connect(&nm, &NotificationManager::userSwitchTask, this, &MainWindow::onNotifSwitch);
    connect(&nm, &NotificationManager::userTakeBreak,  this, &MainWindow::onNotifBreak);
    connect(&nm, &NotificationManager::userContinued,  &nm, &NotificationManager::reset);

    /* sync initial state */
    onStateChanged(tm.state());
    onTodayChanged(tm.todayWorkSeconds());
}

/* ---- Style ------------------------------------------------------- */
void MainWindow::applyStyle()
{
    setStyleSheet(QStringLiteral(R"(
        QWidget {
            background:#1a1a2e; color:#e8e4df;
            font-family:"Ubuntu","Cantarell","Noto Sans",sans-serif; font-size:13px;
        }
        QLineEdit#taskInput {
            background:#16213e; border:1px solid #2a2a4a; border-radius:4px;
            padding:7px 10px; color:#e8e4df;
        }
        QLineEdit#taskInput:focus   { border-color:#ff6b35; }
        QLineEdit#taskInput:disabled{ background:#0f0f1a; color:#5a5665; }

        QPushButton {
            background:#16213e; border:1px solid #2a2a4a; border-radius:4px;
            padding:6px 12px; color:#e8e4df; font-size:12px;
        }
        QPushButton:hover   { background:#2a2a4a; border-color:#ff6b35; }
        QPushButton:disabled{ color:#3a3a4a; border-color:#1a1a2e; }

        QPushButton#startBtn { background:#2d7d46; color:#fff; font-weight:bold; }
        QPushButton#startBtn:hover { background:#3a9d56; }
        QPushButton#stopBtn  { background:#c0392b; color:#fff; font-weight:bold; }
        QPushButton#stopBtn:hover  { background:#e74c3c; }
        QPushButton#breakBtn { background:#8b6914; color:#fff; }
        QPushButton#breakBtn:hover { background:#ab8924; }
        QPushButton#flatBtn  { background:transparent; border:1px solid #2a2a4a; padding:4px 10px; font-size:11px; }

        QLabel#timerLbl  { font-family:"Ubuntu Mono","Courier New",monospace;
                           font-size:18px; font-weight:bold; color:#ff6b35; }
        QLabel#statusLbl { color:#7a7a9a; font-size:12px; }
        QLabel#todayLbl  { color:#5a8a6a; font-size:12px; }
    )"));
}

void MainWindow::refreshWindowFlags()
{
    Qt::WindowFlags f = windowFlags();
    const bool onTop = Config::instance().alwaysOnTop();
    if (onTop) f |=  Qt::WindowStaysOnTopHint;
    else       f &= ~Qt::WindowStaysOnTopHint;
    if (f != windowFlags()) { setWindowFlags(f); if (isVisible()) show(); }
}

/* ---- Events ------------------------------------------------------ */
void MainWindow::closeEvent(QCloseEvent *ev) { toTray(); ev->ignore(); }

void MainWindow::toTray()
{
    hide();
    m_tray->showMessage(QStringLiteral("TimeTracker"),
        QStringLiteral("Running in the background."),
        QSystemTrayIcon::Information, 2000);
}

/* ---- Slots ------------------------------------------------------- */
void MainWindow::onStartStop()
{
    auto &tm = TaskManager::instance();
    if (tm.state() == TaskManager::Idle) {
        const QString d = m_input->text().trimmed();
        if (d.isEmpty()) { m_input->setFocus(); return; }
        tm.startTask(d);
        NotificationManager::instance().start();
    } else {
        tm.stopTask();
        NotificationManager::instance().stop();
    }
}

void MainWindow::onBreak()
{
    auto &tm = TaskManager::instance();
    if (tm.state() == TaskManager::Running) {
        tm.startBreak();
        NotificationManager::instance().stop();
    } else if (tm.state() == TaskManager::Paused) {
        tm.endBreak();
        NotificationManager::instance().start();
    }
}

void MainWindow::onInputReturn()
{
    auto &tm = TaskManager::instance();
    if (tm.state() == TaskManager::Idle) {
        onStartStop();
    } else {
        const QString d = m_input->text().trimmed();
        if (!d.isEmpty() && d != tm.currentDescription()) {
            tm.switchTask(d);
            NotificationManager::instance().reset();
        }
    }
}

void MainWindow::onTick(int) { m_timerLbl->setText(TaskManager::instance().formattedElapsed()); }

void MainWindow::onStateChanged(TaskManager::State s)
{
    switch (s) {
    case TaskManager::Idle:
        m_startBtn->setText(QStringLiteral("Start"));
        m_startBtn->setObjectName(QStringLiteral("startBtn"));
        m_breakBtn->setEnabled(false);
        m_breakBtn->setText(QStringLiteral("Break"));
        m_input->setEnabled(true); m_input->clear();
        m_statusLbl->setText(QStringLiteral("Idle"));
        m_timerLbl->setText(QStringLiteral("00:00:00"));
        m_input->setFocus();
        NotificationManager::instance().stop();
        break;
    case TaskManager::Running:
        m_startBtn->setText(QStringLiteral("Stop"));
        m_startBtn->setObjectName(QStringLiteral("stopBtn"));
        m_breakBtn->setEnabled(true);
        m_breakBtn->setText(QStringLiteral("Break"));
        m_input->setEnabled(true);
        m_input->setText(TaskManager::instance().currentDescription());
        m_statusLbl->setText(QStringLiteral("Working: ") + TaskManager::instance().currentDescription());
        NotificationManager::instance().start();
        break;
    case TaskManager::Paused:
        m_startBtn->setText(QStringLiteral("Stop"));
        m_startBtn->setObjectName(QStringLiteral("stopBtn"));
        m_breakBtn->setEnabled(true);
        m_breakBtn->setText(QStringLiteral("Resume"));
        m_statusLbl->setText(QStringLiteral("On break"));
        NotificationManager::instance().stop();
        break;
    }
    /* re-polish for dynamic objectName changes */
    m_startBtn->style()->unpolish(m_startBtn);
    m_startBtn->style()->polish(m_startBtn);
}

void MainWindow::onTodayChanged(int)
{
    m_todayLbl->setText(QStringLiteral("Today: ") + TaskManager::instance().formattedTodayTotal());
}

void MainWindow::onSleep()
{
    NotificationManager::instance().showNotification(
        QStringLiteral("Sleep Detected"),
        QStringLiteral("System woke from sleep — timer paused."));
    m_tray->showMessage(QStringLiteral("Sleep Detected"),
        QStringLiteral("Timer paused. Click Resume to continue."),
        QSystemTrayIcon::Warning, 8000);
}

void MainWindow::onNotifSwitch()
{
    showNormal(); activateWindow();
    m_input->setFocus(); m_input->selectAll();
}

void MainWindow::onNotifBreak()
{
    TaskManager::instance().startBreak();
    NotificationManager::instance().stop();
}

void MainWindow::showHistory()
{
    HistoryDialog dlg(this);
    dlg.exec();
}

void MainWindow::showSettings()
{
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        refreshWindowFlags();
        setWindowOpacity(Config::instance().windowOpacity());
    }
}

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason r)
{
    if (r == QSystemTrayIcon::Trigger || r == QSystemTrayIcon::DoubleClick) {
        isVisible() ? hide() : (showNormal(), activateWindow());
    }
}
