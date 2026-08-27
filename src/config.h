/*
 * File: src/config.h
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

class Config : public QObject
{
    Q_OBJECT

public:
    static Config &instance();

    void load();
    void save();

    /* ---- Notification ---- */
    int  notificationIntervalMinutes() const          { return m_notifyInterval; }
    void setNotificationIntervalMinutes(int v)        { m_notifyInterval = v; emit changed(); }

    bool notificationsEnabled() const                 { return m_notifyEnabled; }
    void setNotificationsEnabled(bool v)              { m_notifyEnabled = v; emit changed(); }

    bool flashOnNotification() const                  { return m_flashOnNotify; }
    void setFlashOnNotification(bool v)               { m_flashOnNotify = v; emit changed(); }

    int  autoDismissSeconds() const                   { return m_autoDismiss; }
    void setAutoDismissSeconds(int v)                 { m_autoDismiss = v; emit changed(); }

    /* ---- Window ---- */
    bool alwaysOnTop() const                          { return m_alwaysOnTop; }
    void setAlwaysOnTop(bool v)                       { m_alwaysOnTop = v; emit changed(); }

    double windowOpacity() const                      { return m_opacity; }
    void   setWindowOpacity(double v)                 { m_opacity = v; emit changed(); }

    /* ---- Startup ---- */
    bool autoStart() const                            { return m_autoStart; }
    void setAutoStart(bool v);

    bool startMinimized() const                       { return m_startMin; }
    void setStartMinimized(bool v)                    { m_startMin = v; emit changed(); }

    /* ---- Sleep ---- */
    bool autoPauseOnSleep() const                     { return m_autoPause; }
    void setAutoPauseOnSleep(bool v)                  { m_autoPause = v; emit changed(); }

    int  sleepThresholdSeconds() const                { return m_sleepThresh; }
    void setSleepThresholdSeconds(int v)              { m_sleepThresh = v; emit changed(); }

    /* ---- Database ---- */
    QString databasePath() const                      { return m_dbPath; }
    void    setDatabasePath(const QString &v)         { m_dbPath = v; emit changed(); }

signals:
    void changed();

private:
    Config();
    ~Config() override = default;
    Config(const Config &)            = delete;
    Config &operator=(const Config &) = delete;

    void installAutoStartDesktop(bool enable);

    QSettings *m_settings = nullptr;

    int     m_notifyInterval = 5;
    bool    m_notifyEnabled  = true;
    bool    m_flashOnNotify  = true;
    int     m_autoDismiss    = 30;

    bool    m_alwaysOnTop    = true;
    double  m_opacity        = 0.95;

    bool    m_autoStart      = true;
    bool    m_startMin       = false;

    bool    m_autoPause      = true;
    int     m_sleepThresh    = 10;

    QString m_dbPath;
};

#endif // CONFIG_H
