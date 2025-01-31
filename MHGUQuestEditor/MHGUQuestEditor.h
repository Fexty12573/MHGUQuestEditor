#pragma once

#include <map>
#include <QStringListModel>
#include <QtWidgets/QMainWindow>
#include "ui_MHGUQuestEditor.h"
#include "Widgets/EmSetListEditor/EmSetListEditor.h"
#include "Widgets/BossSetEditor/BossSetEditor.h"
#include "Resources/Arc.h"
#include "Resources/Gmd.h"
#include "Resources/QuestArc.h"
#include "Resources/QuestData.h"
#include "Resources/QuestLink.h"
#include "Resources/Rem.h"


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
    void closeEvent(QCloseEvent* event) override;

private:
    void initIconDropdowns();
    void initStatDropdowns() const;
    void initMonsterDropdowns() const;
    void initQuestTypeDropdown() const;
    void initQuestSubTypeDropdown() const;
    void initQuestLevelDropdown() const;
    void initMonsterLevelDropdown() const;
    void initMapDropdown();
    void initSpawnTypeDropdown() const;
    void initBgmDropdown() const;
    void initRequirementsDropdowns() const;
    void initClearTypeDropdown() const;
    void initObjectiveDropdowns() const;
    void initItemLevelDropdowns() const;
    void initItemNames();
    void initSpawns();

    void addRemEntry(const QString& remName, s32 tabIndex);

    void loadSettings();
    void saveSettings() const;

    void onOpenFile();
    void onSaveFile();
    void onSaveFileAs();

    void loadFile(const QString& path);
    void loadQuestArc();
    void saveQuestArc(const QString& path = {});
    void saveQuestFile(const QString& path = {}) const;
    void saveQuestArcToQuestList() const;
    void loadQuestDataIntoUi();
    void loadRemIntoUi(const Resources::Rem& rem, const QString& remName, s32 tabIndex);
    void saveQuestDataFromUi();
    void saveQuestInfoFromUi();
    void saveRemFromUi(Resources::Rem& rem, const QString& remName, s32 tabIndex);

    void openSettings();

private:
    Ui::MHGUQuestEditorClass ui;

    QString openedFile;
    std::unique_ptr<Resources::QuestArc> arc;
    std::vector<Resources::Gmd> gmds;
    std::vector<Resources::Rem> rems;
    std::unique_ptr<Resources::QuestLink> questLink;
    Resources::QuestData questData;
    std::vector<QPixmap> monsterIcons;
    QStringList itemNames;
    QStringListModel* itemNamesModel;
    std::array<EmSetListEditor*, 3> emSetListEditors;
    std::array<BossSetEditor*, 5> bossSetEditors;
    
    std::map<int, QString> mapNames;

    QString questListPath;
    bool autoUpdateQuestList;
    QStringList recentFiles;
    constexpr static int maxRecentFiles = 10;
    QMenu* recentFilesMenu;
};
