#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MHGUQuestEditor.h"

class MHGUQuestEditor : public QMainWindow
{
    Q_OBJECT

public:
    MHGUQuestEditor(QWidget *parent = nullptr);
    ~MHGUQuestEditor();

private:
    Ui::MHGUQuestEditorClass ui;

    std::vector<QPixmap> monsterIcons;
};
