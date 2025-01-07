#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MHGUQuestEditor.h"
#include "Resources/Arc.h"
#include "Resources/Gmd.h"
#include "Resources/QuestArc.h"
#include "Resources/QuestData.h"

class MHGUQuestEditor : public QMainWindow
{
    Q_OBJECT

public:
    MHGUQuestEditor(QWidget *parent = nullptr);
    ~MHGUQuestEditor();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void initIconDropdowns();
    void initStatDropdowns() const;
    void initMonsterDropdowns() const;
    void initQuestTypeDropdown() const;
    void initQuestSubTypeDropdown() const;
    void initQuestLevelDropdown() const;
    void initMonsterLevelDropdown() const;
    void initMapDropdown() const;
    void initSpawnTypeDropdown() const;
    void initBgmDropdown() const;
    void initRequirementsDropdowns() const;
    void initClearTypeDropdown() const;
    void initObjectiveDropdowns() const;
    void initItemLevelDropdowns() const;
    void initItemNames();

    void onOpenFile();
    void onSaveFile();

    void loadFile(const QString& path);
    void loadQuestArc();
    void loadQuestDataIntoUi();
    void saveQuestDataFromUi();

private:
    Ui::MHGUQuestEditorClass ui;

    std::unique_ptr<Resources::QuestArc> arc;
    std::vector<Resources::Gmd> gmds;
    Resources::QuestData questData;
    std::vector<QPixmap> monsterIcons;
    QStringList itemNames;
};
