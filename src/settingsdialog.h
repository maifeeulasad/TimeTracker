/*
 * File: src/settingsdialog.h
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QLineEdit;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void save();
    void browseDb();

private:
    void buildUi();
    void loadCurrent();

    QCheckBox      *m_notifyOn    = nullptr;
    QSpinBox       *m_notifyInt   = nullptr;
    QCheckBox      *m_flashOn     = nullptr;
    QSpinBox       *m_autoDismiss = nullptr;

    QCheckBox      *m_onTop       = nullptr;
    QDoubleSpinBox *m_opacity     = nullptr;

    QCheckBox      *m_autoStart   = nullptr;
    QCheckBox      *m_startMin    = nullptr;

    QCheckBox      *m_autoPause   = nullptr;
    QSpinBox       *m_sleepThr    = nullptr;

    QLineEdit      *m_dbPath      = nullptr;
};

#endif // SETTINGSDIALOG_H
