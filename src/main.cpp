/*
 * File: src/main.cpp
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include <QCommandLineParser>

#include "config.h"
#include "databasemanager.h"
#include "taskmanager.h"
#include "notificationmanager.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("TimeTracker"));
    app.setOrganizationName(QStringLiteral("MaifeeUlAsad"));
    app.setOrganizationDomain(QStringLiteral("maifee.dev"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    /* ---- CLI ---- */
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Persistent time tracker for Ubuntu"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(QCommandLineOption(QStringLiteral("minimized"),
                     QStringLiteral("Start minimised to system tray")));
    parser.process(app);

    /* ---- single instance ---- */
    QSharedMemory guard(QStringLiteral("TimeTracker_SingleInstance_v1"));
    if (!guard.create(1)) {
        QMessageBox::warning(nullptr, QStringLiteral("TimeTracker"),
            QStringLiteral("Another instance is already running."));
        return 0;
    }

    /* ---- config ---- */
    Config::instance().load();

    /* ---- database ---- */
    if (!DatabaseManager::instance().initialize()) {
        QMessageBox::critical(nullptr, QStringLiteral("TimeTracker"),
            QStringLiteral("Failed to initialise the database.\nCheck disk space and permissions."));
        return 1;
    }

    /* ---- managers (order matters) ---- */
    TaskManager::instance();           // may resume an active task
    NotificationManager::instance();

    /* ---- main window ---- */
    MainWindow win;
    win.setWindowOpacity(Config::instance().windowOpacity());

    if (parser.isSet(QStringLiteral("minimized")) || Config::instance().startMinimized()) {
        /* tray-only launch */
    } else {
        win.show();
    }

    const int ret = app.exec();

    /* ---- cleanup ---- */
    auto &tm = TaskManager::instance();
    if (tm.state() != TaskManager::Idle) tm.stopTask();
    Config::instance().save();

    return ret;
}
