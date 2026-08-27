/*
 * File: src/config.cpp
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#include "config.h"

#include <QFile>
#include <QTextStream>
#include <QDir>

/* ------------------------------------------------------------------ */
Config::Config()
{
    const QString cfgDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(cfgDir);
    m_settings = new QSettings(cfgDir + QStringLiteral("/timetracker.conf"),
                               QSettings::IniFormat, this);

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_dbPath = dataDir + QStringLiteral("/timetracker.db");
}

Config &Config::instance()
{
    static Config inst;
    return inst;
}

/* ------------------------------------------------------------------ */
void Config::load()
{
    m_notifyInterval = m_settings->value(QStringLiteral("notification/interval_minutes"), 5).toInt();
    m_notifyEnabled  = m_settings->value(QStringLiteral("notification/enabled"),          true).toBool();
    m_flashOnNotify  = m_settings->value(QStringLiteral("notification/flash_window"),     true).toBool();
    m_autoDismiss    = m_settings->value(QStringLiteral("notification/auto_dismiss_sec"), 30).toInt();

    m_alwaysOnTop    = m_settings->value(QStringLiteral("window/always_on_top"), true).toBool();
    m_opacity        = m_settings->value(QStringLiteral("window/opacity"),       0.95).toDouble();

    m_autoStart      = m_settings->value(QStringLiteral("startup/auto_start"),      true).toBool();
    m_startMin       = m_settings->value(QStringLiteral("startup/start_minimized"), false).toBool();

    m_autoPause      = m_settings->value(QStringLiteral("sleep/auto_pause"),       true).toBool();
    m_sleepThresh    = m_settings->value(QStringLiteral("sleep/threshold_seconds"), 10).toInt();

    m_dbPath         = m_settings->value(QStringLiteral("database/path"), m_dbPath).toString();
}

void Config::save()
{
    m_settings->setValue(QStringLiteral("notification/interval_minutes"), m_notifyInterval);
    m_settings->setValue(QStringLiteral("notification/enabled"),          m_notifyEnabled);
    m_settings->setValue(QStringLiteral("notification/flash_window"),     m_flashOnNotify);
    m_settings->setValue(QStringLiteral("notification/auto_dismiss_sec"), m_autoDismiss);

    m_settings->setValue(QStringLiteral("window/always_on_top"), m_alwaysOnTop);
    m_settings->setValue(QStringLiteral("window/opacity"),       m_opacity);

    m_settings->setValue(QStringLiteral("startup/auto_start"),      m_autoStart);
    m_settings->setValue(QStringLiteral("startup/start_minimized"), m_startMin);

    m_settings->setValue(QStringLiteral("sleep/auto_pause"),       m_autoPause);
    m_settings->setValue(QStringLiteral("sleep/threshold_seconds"), m_sleepThresh);

    m_settings->setValue(QStringLiteral("database/path"), m_dbPath);
    m_settings->sync();
}

/* ------------------------------------------------------------------ */
void Config::setAutoStart(bool v)
{
    m_autoStart = v;
    installAutoStartDesktop(v);
    emit changed();
}

void Config::installAutoStartDesktop(bool enable)
{
    const QString dir  = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
                         + QStringLiteral("/autostart");
    const QString file = dir + QStringLiteral("/timetracker.desktop");

    if (enable) {
        QDir().mkpath(dir);
        QFile f(file);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream ts(&f);
            ts << "[Desktop Entry]\n"
               << "Type=Application\n"
               << "Name=TimeTracker\n"
               << "Exec=timetracker --minimized\n"
               << "Terminal=false\n"
               << "X-GNOME-Autostart-enabled=true\n";
        }
    } else {
        QFile::remove(file);
    }
}
