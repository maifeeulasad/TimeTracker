/*
 * File: src/historydialog.h
 * TimeTracker — Persistent time tracking for Ubuntu
 * Author: Maifee Ul Asad
 * License: MIT
 */

#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>

class QDateEdit;
class QLineEdit;
class QTableWidget;
class QLabel;

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = nullptr);

private slots:
    void refresh();
    void doExport();

private:
    void buildUi();
    void populate();
    void recalcSummary();

    QDateEdit    *m_from  = nullptr;
    QDateEdit    *m_to    = nullptr;
    QLineEdit    *m_search = nullptr;
    QTableWidget *m_table  = nullptr;
    QLabel       *m_sumLbl = nullptr;
    QLabel       *m_cntLbl = nullptr;
};

#endif // HISTORYDIALOG_H
