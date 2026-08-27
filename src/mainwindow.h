/*
 * File: src/mainwindow.h
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QSystemTrayIcon>

#include "taskmanager.h"

class QLineEdit;
class QPushButton;
class QLabel;
class QMenu;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *ev) override;

private slots:
    void onStartStop();
    void onBreak();
    void onInputReturn();
    void onTick(int secs);
    void onStateChanged(TaskManager::State s);
    void onTodayChanged(int secs);
    void onSleep();
    void onNotifSwitch();
    void onNotifBreak();
    void showHistory();
    void showSettings();
    void trayActivated(QSystemTrayIcon::ActivationReason r);

private:
    void buildUi();
    void buildTray();
    void connectSignals();
    void applyStyle();
    void refreshWindowFlags();
    void toTray();

    QLineEdit        *m_input      = nullptr;
    QPushButton      *m_startBtn   = nullptr;
    QPushButton      *m_breakBtn   = nullptr;
    QLabel           *m_timerLbl   = nullptr;
    QLabel           *m_statusLbl  = nullptr;
    QLabel           *m_todayLbl   = nullptr;
    QPushButton      *m_histBtn    = nullptr;
    QPushButton      *m_settBtn    = nullptr;
    QPushButton      *m_minBtn     = nullptr;
    QSystemTrayIcon  *m_tray       = nullptr;
    QMenu            *m_trayMenu   = nullptr;
};

#endif // MAINWINDOW_H
