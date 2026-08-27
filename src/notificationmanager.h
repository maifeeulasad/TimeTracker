/*
 * File: src/notificationmanager.h
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QSystemTrayIcon>

class QWidget;
class QDialog;

class NotificationManager : public QObject
{
    Q_OBJECT

public:
    static NotificationManager &instance();

    void setMainWindow(QWidget *w);
    void start();
    void stop();
    void reset();

    void showNotification(const QString &title, const QString &body);

public slots:
    void onConfigChanged();

signals:
    void notificationFired();
    void userContinued();
    void userSwitchTask();
    void userTakeBreak();

private:
    NotificationManager();
    ~NotificationManager() override = default;
    NotificationManager(const NotificationManager &)            = delete;
    NotificationManager &operator=(const NotificationManager &) = delete;

    void fireNotification();
    void showCheckDialog();

    QTimer            *m_timer      = nullptr;
    QWidget           *m_mainWin    = nullptr;
    QSystemTrayIcon   *m_tray       = nullptr;
    QDialog           *m_activeDlg  = nullptr;
};

#endif // NOTIFICATIONMANAGER_H
