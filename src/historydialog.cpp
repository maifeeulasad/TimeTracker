/*
 * File: src/historydialog.cpp
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#include "historydialog.h"
#include "databasemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>

/* ================================================================== */
HistoryDialog::HistoryDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("TimeTracker — History"));
    setMinimumSize(740, 520);
    buildUi();
    populate();
}

void HistoryDialog::buildUi()
{
    auto *root = new QVBoxLayout(this); root->setSpacing(10);

    /* ---- filter row ---- */
    auto *fr = new QHBoxLayout; fr->setSpacing(8);
    fr->addWidget(new QLabel(QStringLiteral("From:")));
    m_from = new QDateEdit(QDate::currentDate().addDays(-7));
    m_from->setCalendarPopup(true); fr->addWidget(m_from);
    fr->addWidget(new QLabel(QStringLiteral("To:")));
    m_to = new QDateEdit(QDate::currentDate());
    m_to->setCalendarPopup(true); fr->addWidget(m_to);
    fr->addSpacing(10);
    fr->addWidget(new QLabel(QStringLiteral("Search:")));
    m_search = new QLineEdit;
    m_search->setPlaceholderText(QStringLiteral("Filter by task name…"));
    fr->addWidget(m_search, 1);
    auto *sBtn = new QPushButton(QStringLiteral("Search"));
    fr->addWidget(sBtn);
    root->addLayout(fr);

    /* ---- table ---- */
    m_table = new QTableWidget;
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels(
        {QStringLiteral("Date"), QStringLiteral("Task"), QStringLiteral("Start"),
         QStringLiteral("End"), QStringLiteral("Duration"), QStringLiteral("Status")});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet(QStringLiteral(
        "QTableWidget{background:#16213e;alternate-background-color:#1a1a2e;"
        "color:#e8e4df;gridline-color:#2a2a4a;border:1px solid #2a2a4a;border-radius:4px;}"
        "QHeaderView::section{background:#0f0f1a;color:#ff6b35;padding:6px;"
        "border:none;border-bottom:2px solid #ff6b35;font-weight:bold;}"
        "QTableWidget::item:selected{background:#2a5298;}"));
    root->addWidget(m_table, 1);

    /* ---- summary ---- */
    auto *sr = new QHBoxLayout;
    m_sumLbl = new QLabel; m_cntLbl = new QLabel;
    sr->addWidget(m_sumLbl, 1); sr->addWidget(m_cntLbl);
    root->addLayout(sr);

    /* ---- buttons ---- */
    auto *br = new QHBoxLayout;
    auto *expBtn = new QPushButton(QStringLiteral("Export CSV"));
    auto *clsBtn = new QPushButton(QStringLiteral("Close"));
    br->addStretch(); br->addWidget(expBtn); br->addWidget(clsBtn);
    root->addLayout(br);

    connect(sBtn,   &QPushButton::clicked, this, &HistoryDialog::refresh);
    connect(m_search, &QLineEdit::returnPressed, this, &HistoryDialog::refresh);
    connect(expBtn, &QPushButton::clicked, this, &HistoryDialog::doExport);
    connect(clsBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_from, &QDateEdit::dateChanged, this, &HistoryDialog::refresh);
    connect(m_to,   &QDateEdit::dateChanged, this, &HistoryDialog::refresh);
}

/* ------------------------------------------------------------------ */
static QString fmtSecs(int sec)
{
    return QStringLiteral("%1:%2:%3")
        .arg(sec / 3600, 2, 10, QLatin1Char('0'))
        .arg((sec % 3600) / 60, 2, 10, QLatin1Char('0'))
        .arg(sec % 60, 2, 10, QLatin1Char('0'));
}

void HistoryDialog::populate()
{
    auto &db = DatabaseManager::instance();
    auto tasks = db.getTasksByDateRange(m_from->date(), m_to->date());

    const QString needle = m_search->text().trimmed().toLower();
    if (!needle.isEmpty()) {
        QList<TaskRecord> filtered;
        for (const auto &t : tasks)
            if (t.description.toLower().contains(needle)) filtered.append(t);
        tasks = filtered;
    }

    m_table->setRowCount(tasks.size());
    for (int i = 0; i < tasks.size(); ++i) {
        const auto &t = tasks[i];
        m_table->setItem(i, 0, new QTableWidgetItem(t.startedAt.date().toString(QStringLiteral("yyyy-MM-dd"))));
        m_table->setItem(i, 1, new QTableWidgetItem(t.description));
        m_table->setItem(i, 2, new QTableWidgetItem(t.startedAt.time().toString(QStringLiteral("HH:mm:ss"))));
        m_table->setItem(i, 3, new QTableWidgetItem(
            t.endedAt.isValid() ? t.endedAt.time().toString(QStringLiteral("HH:mm:ss")) : QStringLiteral("-")));

        int dur = t.durationSeconds;
        if (dur <= 0 && t.endedAt.isValid())
            dur = static_cast<int>(t.startedAt.secsTo(t.endedAt));
        m_table->setItem(i, 4, new QTableWidgetItem(fmtSecs(dur)));

        auto *si = new QTableWidgetItem(t.status);
        if      (t.status == QLatin1String("active"))    si->setForeground(QColor("#ff6b35"));
        else if (t.status == QLatin1String("completed")) si->setForeground(QColor("#2d7d46"));
        else if (t.status == QLatin1String("cancelled")) si->setForeground(QColor("#c0392b"));
        m_table->setItem(i, 5, si);
    }
    recalcSummary();
}

void HistoryDialog::recalcSummary()
{
    int total = 0;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        const QStringList p = m_table->item(i, 4)->text().split(QLatin1Char(':'));
        if (p.size() == 3) total += p[0].toInt() * 3600 + p[1].toInt() * 60 + p[2].toInt();
    }
    m_sumLbl->setText(QStringLiteral("Total: %1h %2m")
        .arg(total / 3600).arg((total % 3600) / 60, 2, 10, QLatin1Char('0')));
    m_cntLbl->setText(QStringLiteral("%1 tasks").arg(m_table->rowCount()));
}

void HistoryDialog::refresh() { populate(); }

void HistoryDialog::doExport()
{
    const QString def = QStringLiteral("timetracker_%1_%2.csv")
        .arg(m_from->date().toString(QStringLiteral("yyyyMMdd")),
             m_to->date().toString(QStringLiteral("yyyyMMdd")));
    const QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("Export CSV"), def, QStringLiteral("CSV (*.csv)"));
    if (path.isEmpty()) return;

    if (DatabaseManager::instance().exportToCsv(path, m_from->date(), m_to->date()))
        QMessageBox::information(this, QStringLiteral("Export"),
            QStringLiteral("Exported to:\n") + path);
    else
        QMessageBox::warning(this, QStringLiteral("Error"), QStringLiteral("Export failed."));
}
