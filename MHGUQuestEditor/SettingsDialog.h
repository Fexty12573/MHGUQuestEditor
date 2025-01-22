#pragma once

#include <QDialog>
#include <QSettings>
#include "ui_SettingsDialog.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget* parent = nullptr, QString questList = {}, bool autoUpdate = false);
    ~SettingsDialog();

    QString getQuestListPath() const { return questListPath; }
    bool getAutoUpdateQuestList() const { return autoUpdateQuestList; }

private:
    void browseForQuestList();

private:
    Ui::SettingsDialog ui;

    QString questListPath;
    bool autoUpdateQuestList;
};
