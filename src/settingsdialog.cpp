/*
 * File: src/settingsdialog.cpp
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#include "settingsdialog.h"
#include "config.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>

/* ================================================================== */
SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("TimeTracker — Settings"));
    setMinimumWidth(440);
    buildUi();
    loadCurrent();
}

void SettingsDialog::buildUi()
{
    auto *root = new QVBoxLayout(this); root->setSpacing(12);

    /* ---- Notifications ---- */
    auto *ng = new QGroupBox(QStringLiteral("Notifications"));
    auto *nf = new QFormLayout(ng);
    m_notifyOn    = new QCheckBox(QStringLiteral("Enable periodic check-ins"));
    m_notifyInt   = new QSpinBox; m_notifyInt->setRange(1, 120); m_notifyInt->setSuffix(QStringLiteral(" min"));
    m_flashOn     = new QCheckBox(QStringLiteral("Flash window on notification"));
    m_autoDismiss = new QSpinBox; m_autoDismiss->setRange(0, 300);
    m_autoDismiss->setSuffix(QStringLiteral(" sec")); m_autoDismiss->setSpecialValueText(QStringLiteral("Manual"));
    nf->addRow(m_notifyOn);
    nf->addRow(QStringLiteral("Interval:"), m_notifyInt);
    nf->addRow(m_flashOn);
    nf->addRow(QStringLiteral("Auto-dismiss:"), m_autoDismiss);
    root->addWidget(ng);

    /* ---- Window ---- */
    auto *wg = new QGroupBox(QStringLiteral("Window"));
    auto *wf = new QFormLayout(wg);
    m_onTop   = new QCheckBox(QStringLiteral("Always on top"));
    m_opacity = new QDoubleSpinBox; m_opacity->setRange(0.3, 1.0);
    m_opacity->setSingleStep(0.05); m_opacity->setDecimals(2);
    wf->addRow(m_onTop);
    wf->addRow(QStringLiteral("Opacity:"), m_opacity);
    root->addWidget(wg);

    /* ---- Startup ---- */
    auto *sg = new QGroupBox(QStringLiteral("Startup"));
    auto *sf = new QFormLayout(sg);
    m_autoStart = new QCheckBox(QStringLiteral("Start on login"));
    m_startMin  = new QCheckBox(QStringLiteral("Start minimised to tray"));
    sf->addRow(m_autoStart);
    sf->addRow(m_startMin);
    root->addWidget(sg);

    /* ---- Sleep ---- */
    auto *slg = new QGroupBox(QStringLiteral("Sleep Detection"));
    auto *slf = new QFormLayout(slg);
    m_autoPause = new QCheckBox(QStringLiteral("Auto-pause on sleep / wake"));
    m_sleepThr  = new QSpinBox; m_sleepThr->setRange(5, 120); m_sleepThr->setSuffix(QStringLiteral(" sec"));
    slf->addRow(m_autoPause);
    slf->addRow(QStringLiteral("Threshold:"), m_sleepThr);
    root->addWidget(slg);

    /* ---- Database ---- */
    auto *dg = new QGroupBox(QStringLiteral("Database"));
    auto *dh = new QHBoxLayout(dg);
    m_dbPath = new QLineEdit; m_dbPath->setReadOnly(true);
    auto *browse = new QPushButton(QStringLiteral("Browse…"));
    connect(browse, &QPushButton::clicked, this, &SettingsDialog::browseDb);
    dh->addWidget(m_dbPath, 1); dh->addWidget(browse);
    root->addWidget(dg);

    /* ---- Buttons ---- */
    auto *bb = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(bb, &QDialogButtonBox::accepted, this, &SettingsDialog::save);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(bb);
}

void SettingsDialog::loadCurrent()
{
    const auto &c = Config::instance();
    m_notifyOn->setChecked(c.notificationsEnabled());
    m_notifyInt->setValue(c.notificationIntervalMinutes());
    m_flashOn->setChecked(c.flashOnNotification());
    m_autoDismiss->setValue(c.autoDismissSeconds());
    m_onTop->setChecked(c.alwaysOnTop());
    m_opacity->setValue(c.windowOpacity());
    m_autoStart->setChecked(c.autoStart());
    m_startMin->setChecked(c.startMinimized());
    m_autoPause->setChecked(c.autoPauseOnSleep());
    m_sleepThr->setValue(c.sleepThresholdSeconds());
    m_dbPath->setText(c.databasePath());
}

void SettingsDialog::save()
{
    auto &c = Config::instance();
    c.setNotificationsEnabled(m_notifyOn->isChecked());
    c.setNotificationIntervalMinutes(m_notifyInt->value());
    c.setFlashOnNotification(m_flashOn->isChecked());
    c.setAutoDismissSeconds(m_autoDismiss->value());
    c.setAlwaysOnTop(m_onTop->isChecked());
    c.setWindowOpacity(m_opacity->value());
    c.setAutoStart(m_autoStart->isChecked());
    c.setStartMinimized(m_startMin->isChecked());
    c.setAutoPauseOnSleep(m_autoPause->isChecked());
    c.setSleepThresholdSeconds(m_sleepThr->value());
    c.setDatabasePath(m_dbPath->text());
    c.save();
    accept();
}

void SettingsDialog::browseDb()
{
    const QString p = QFileDialog::getSaveFileName(
        this, QStringLiteral("Database Location"), m_dbPath->text(),
        QStringLiteral("SQLite (*.db)"));
    if (!p.isEmpty()) m_dbPath->setText(p);
}
