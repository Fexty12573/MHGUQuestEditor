#include "MHGUQuestEditor.h"

#include <ranges>
#include <regex>

#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QSettings>
#include <QRandomGenerator>

#include "SettingsDialog.h"
#include "Resources/Arc.h"
#include "Resources/QuestData.h"
#include "Resources/StatTable.h"
#include "Util/Crc32.h"
#include <QListView>

template<typename T> concept Enum = std::is_enum_v<T>;
template<typename T> concept Integral = std::is_integral_v<T>;
template<typename T> concept AnyIntegral = Integral<T> || Enum<T>;

MHGUQuestEditor::MHGUQuestEditor(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    setAcceptDrops(true);
    setWindowIcon(QIcon(":/res/icon.png"));

    // Initialize dropdowns
    initIconDropdowns();
    initStatDropdowns();
    initMonsterDropdowns();
    initQuestTypeDropdown();
    initQuestSubTypeDropdown();
    initQuestLevelDropdown();
    initMonsterLevelDropdown();
    initMapDropdown();
    initSpawnTypeDropdown();
    initBgmDropdown();
    initRequirementsDropdowns();
    initClearTypeDropdown();
    initObjectiveDropdowns();
    initItemLevelDropdowns();
    initItemNames();

    loadSettings();

    recentFilesMenu = new QMenu("Open Recent", ui.menuFile);
    const QFont font("Segoe UI", 11);
    recentFilesMenu->setFont(font);
    ui.menuFile->insertMenu(ui.actionSave, recentFilesMenu)->setFont(font);
    ui.menuFile->insertSeparator(ui.actionSave);

    connect(ui.actionOpen, &QAction::triggered, this, &MHGUQuestEditor::onOpenFile);
    connect(ui.actionDuplicateQuestInfo, &QAction::triggered, this, [this] {
        const auto button = QMessageBox::warning(this, "Duplicate Quest Info",
            R"(Warning: This will duplicate the quest info from
the currently selected language into all other languages.
This operation cannot be undone. Do you want to continue?)", QMessageBox::Yes | QMessageBox::No);

        if (button == QMessageBox::Yes)
        {
            using Resources::Language;
            const auto currentLang = ui.tabWidgetLanguage->currentIndex();
            const auto currentLangString = Language::toString(currentLang, true);

            const auto currentWidget = ui.tabWidgetLanguage->currentWidget();
            const auto currentName = currentWidget->findChild<QLineEdit*>("textName" + currentLangString)->text();
            const auto currentClient = currentWidget->findChild<QLineEdit*>("textClient" + currentLangString)->text();
            const auto currentDesc = currentWidget->findChild<QPlainTextEdit*>("textDescription" + currentLangString)->toPlainText();
            const auto currentZako = currentWidget->findChild<QPlainTextEdit*>("textZako" + currentLangString)->toPlainText();
            const auto currentObjective = currentWidget->findChild<QPlainTextEdit*>("textObjective" + currentLangString)->toPlainText();
            const auto currentFailure = currentWidget->findChild<QPlainTextEdit*>("textFailure" + currentLangString)->toPlainText();
            const auto currentSub = currentWidget->findChild<QLineEdit*>("textSubquest" + currentLangString)->text();

            for (auto i = 0; i < Language::Count; i++)
            {
                if (i == currentLang)
                    continue;

                const auto languageString = Language::toString(i, true);
                const auto widget = ui.tabWidgetLanguage->widget(i);
                widget->findChild<QLineEdit*>("textName" + languageString)->setText(currentName);
                widget->findChild<QLineEdit*>("textClient" + languageString)->setText(currentClient);
                widget->findChild<QPlainTextEdit*>("textDescription" + languageString)->setPlainText(currentDesc);
                widget->findChild<QPlainTextEdit*>("textZako" + languageString)->setPlainText(currentZako);
                widget->findChild<QPlainTextEdit*>("textObjective" + languageString)->setPlainText(currentObjective);
                widget->findChild<QPlainTextEdit*>("textFailure" + languageString)->setPlainText(currentFailure);
                widget->findChild<QLineEdit*>("textSubquest" + languageString)->setText(currentSub);
            }
        }
    });
    connect(ui.actionSettings, &QAction::triggered, this, &MHGUQuestEditor::openSettings);
    connect(ui.actionSave, &QAction::triggered, this, &MHGUQuestEditor::onSaveFile);
    connect(ui.actionSaveAs, &QAction::triggered, this, &MHGUQuestEditor::onSaveFileAs);
    connect(ui.tabWidgetRoot, &QTabWidget::currentChanged, this, [this](int index) {
        ui.actionDuplicateQuestInfo->setEnabled(index == 1); // Is in Quest Info tab
    });
    connect(recentFilesMenu, &QMenu::aboutToShow, this, [this, font] {
        recentFilesMenu->clear();
        for (const auto& file : recentFiles)
        {
            const auto action = recentFilesMenu->addAction(file);
            action->setFont(font);
            connect(action, &QAction::triggered, this, [this, file] {
                loadFile(file);
            });
        }
    });
    connect(ui.buttonRemMainAAdd, &QPushButton::pressed, this, [this] { addRemEntry("MainA", 0); });
    connect(ui.buttonRemMainBAdd, &QPushButton::pressed, this, [this] { addRemEntry("MainB", 1); });
    connect(ui.buttonRemExtraAAdd, &QPushButton::pressed, this, [this] { addRemEntry("ExtraA", 2); });
    connect(ui.buttonRemExtraBAdd, &QPushButton::pressed, this, [this] { addRemEntry("ExtraB", 3); });
    connect(ui.buttonRemSubAdd, &QPushButton::pressed, this, [this] { addRemEntry("Sub", 4); });

    ui.tabWidgetRoot->setTabEnabled(1, false); // Quest Info
    ui.tabWidgetRoot->setTabEnabled(2, false); // Rewards
    ui.tabWidgetRoot->setTabEnabled(3, false); // Small Monsters
    ui.tabWidgetRoot->setCurrentIndex(0);
    ui.tabWidgetQuestData->setCurrentIndex(0);
    ui.tabWidgetLanguage->setCurrentIndex(0);
    ui.tabWidgetRewards->setCurrentIndex(0);
    ui.tabWidgetZako->setCurrentIndex(0);

    emSetListEditors = {
        new EmSetListEditor({}, this),
        new EmSetListEditor({}, this),
        new EmSetListEditor({}, this)
    };

    const auto zakoA = ui.tabWidgetZako->widget(0);
    const auto zakoB = ui.tabWidgetZako->widget(1);
    const auto zakoC = ui.tabWidgetZako->widget(2);

    auto layout = new QVBoxLayout(zakoA);
    layout->addWidget(emSetListEditors[0]);
    zakoA->setLayout(layout);

    layout = new QVBoxLayout(zakoB);
    layout->addWidget(emSetListEditors[1]);
    zakoB->setLayout(layout);

    layout = new QVBoxLayout(zakoC);
    layout->addWidget(emSetListEditors[2]);
    zakoC->setLayout(layout);
}

MHGUQuestEditor::~MHGUQuestEditor() = default;

void MHGUQuestEditor::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void MHGUQuestEditor::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void MHGUQuestEditor::dropEvent(QDropEvent* event)
{
    const auto url = event->mimeData()->urls().first();
    loadFile(url.toLocalFile());
}

void MHGUQuestEditor::closeEvent(QCloseEvent* event)
{
    saveSettings();
    event->accept();
}

void MHGUQuestEditor::initIconDropdowns()
{
    const QImage icons(":/res/cmn_micon00.png");
    constexpr int iconSize = 70;
    constexpr int offsetX = 1;
    constexpr int offsetY = 1;
    constexpr int gap = 2;

    for (int y = 0; y < 14; y++)
    {
        for (int x = 0; x < 7; x++)
        {
            const int xPos = x * (iconSize + gap) + offsetX;
            const int yPos = y * (iconSize + gap) + offsetY;
            monsterIcons.emplace_back(QPixmap::fromImage(icons.copy(xPos, yPos, iconSize, iconSize)));
        }
    }

    const QImage icons2(":/res/cmn_micon01.png");

    for (int y = 0; y < 14; y++)
    {
        for (int x = 0; x < 7; x++)
        {
            const int xPos = x * (iconSize + gap) + offsetX;
            const int yPos = y * (iconSize + gap) + offsetY;
            monsterIcons.emplace_back(QPixmap::fromImage(icons2.copy(xPos, yPos, iconSize, iconSize)));

            if (y == 6)
            {
                y = 14;
                break;
            }
        }
    }

    auto iconNames = QFile(":/res/icon_names.json");
    if (!iconNames.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open icon_names.json");
    }

    QJsonArray iconNamesArray = QJsonDocument::fromJson(iconNames.readAll()).array();
    for (int i = 0; i < iconNamesArray.size(); i++)
    {
        const auto name = iconNamesArray[i].toString("N/A");

        if (i == 0 || i >= monsterIcons.size())
        {
            ui.comboIcon1->addItem(name);
            ui.comboIcon2->addItem(name);
            ui.comboIcon3->addItem(name);
            ui.comboIcon4->addItem(name);
            ui.comboIcon5->addItem(name);
        }
        else
        {
            const auto icon = monsterIcons[i - 1];
            ui.comboIcon1->addItem(icon, name);
            ui.comboIcon2->addItem(icon, name);
            ui.comboIcon3->addItem(icon, name);
            ui.comboIcon4->addItem(icon, name);
            ui.comboIcon5->addItem(icon, name);
        }
    }

    connect(ui.comboIcon1, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index == 0 || index >= monsterIcons.size()) {
            ui.labelIcon1->setPixmap({});
            return;
        }
        ui.labelIcon1->setPixmap(monsterIcons[index - 1]);
    });
    connect(ui.comboIcon2, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index == 0 || index >= monsterIcons.size()) {
            ui.labelIcon2->setPixmap({});
            return;
        }
        ui.labelIcon2->setPixmap(monsterIcons[index - 1]);
    });
    connect(ui.comboIcon3, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index == 0 || index >= monsterIcons.size()) {
            ui.labelIcon3->setPixmap({});
            return;
        }
        ui.labelIcon3->setPixmap(monsterIcons[index - 1]);
    });
    connect(ui.comboIcon4, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index == 0 || index >= monsterIcons.size()) {
            ui.labelIcon4->setPixmap({});
            return;
        }
        ui.labelIcon4->setPixmap(monsterIcons[index - 1]);
    });
    connect(ui.comboIcon5, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (index == 0 || index >= monsterIcons.size()) {
            ui.labelIcon5->setPixmap({});
            return;
        }
        ui.labelIcon5->setPixmap(monsterIcons[index - 1]);
    });
}

void MHGUQuestEditor::initStatDropdowns() const {
    QFile file(":/res/em_nando_tbl.nan");
    if (!file.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open em_nando_tbl.nan");
    }

    const Resources::StatTable table = Resources::StatTable::deserialize(file.readAll());
    for (int i = 0; i < table.size(); i++)
    {
        const auto& entry = table[i];

        const auto health = QString("%1%").arg(entry.Health * 100.0f);
        const auto attack = QString("%1%").arg(entry.Attack * 100.0f);
        const auto defense = QString("%1%").arg(entry.Defense * 100.0f);
        const auto other = QString("Stagger: %1% Exhaust: %1% KO: %1% Mount: %1% Unk: %1%")
            .arg(entry.Stagger * 100.0f)
            .arg(entry.Exhaust * 100.0f)
            .arg(entry.KO * 100.0f)
            .arg(entry.Mount * 100.0f)
            .arg(entry.Unknown * 100.0f);

        ui.comboMonster1Health->addItem(health);
        ui.comboMonster1Attack->addItem(attack);
        ui.comboMonster1Defense->addItem(defense);
        ui.comboMonster1Other->addItem(other);

        ui.comboMonster2Health->addItem(health);
        ui.comboMonster2Attack->addItem(attack);
        ui.comboMonster2Defense->addItem(defense);
        ui.comboMonster2Other->addItem(other);

        ui.comboMonster3Health->addItem(health);
        ui.comboMonster3Attack->addItem(attack);
        ui.comboMonster3Defense->addItem(defense);
        ui.comboMonster3Other->addItem(other);

        ui.comboMonster4Health->addItem(health);
        ui.comboMonster4Attack->addItem(attack);
        ui.comboMonster4Defense->addItem(defense);
        ui.comboMonster4Other->addItem(other);

        ui.comboMonster5Health->addItem(health);
        ui.comboMonster5Attack->addItem(attack);
        ui.comboMonster5Defense->addItem(defense);
        ui.comboMonster5Other->addItem(other);

        ui.comboZakoHealth->addItem(health);
        ui.comboZakoAttack->addItem(attack);
        ui.comboZakoOther->addItem(other);
    }
}

void MHGUQuestEditor::initMonsterDropdowns() const {
    auto monsterNames = QFile(":/res/em_names.json");
    if (!monsterNames.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open em_names.json");
    }

    QJsonObject names = QJsonDocument::fromJson(monsterNames.readAll()).object();
    for (const auto& name : names.keys())
    {
        const auto id = QVariant(names[name].toInt());
        ui.comboMonster1->addItem(name, id);
        ui.comboMonster2->addItem(name, id);
        ui.comboMonster3->addItem(name, id);
        ui.comboMonster4->addItem(name, id);
        ui.comboMonster5->addItem(name, id);
    }
}

void MHGUQuestEditor::initQuestTypeDropdown() const {
    auto questTypes = QFile(":/res/quest_type.json");
    if (!questTypes.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open quest_type.json");
    }

    QJsonArray types = QJsonDocument::fromJson(questTypes.readAll()).array();
    for (const auto [i, type] : std::views::enumerate(types))
    {
        ui.comboQuestType->addItem(type.toString(), i);
    }
}

void MHGUQuestEditor::initQuestSubTypeDropdown() const {
    auto questSubTypes = QFile(":/res/quest_subtype.json");
    if (!questSubTypes.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open quest_subtype.json");
    }

    QJsonArray subTypes = QJsonDocument::fromJson(questSubTypes.readAll()).array();
    for (const auto [i, type] : std::views::enumerate(subTypes))
    {
        ui.comboQuestSubType->addItem(type.toString(), i);
    }
}

void MHGUQuestEditor::initQuestLevelDropdown() const {
    auto questLevels = QFile(":/res/quest_level.json");
    if (!questLevels.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open quest_level.json");
    }

    QJsonArray levels = QJsonDocument::fromJson(questLevels.readAll()).array();
    for (const auto level : levels)
    {
        const auto obj = level.toObject();
        ui.comboQuestLevel->addItem(obj["Name"].toString(), obj["Value"].toInt());
    }
}

void MHGUQuestEditor::initMonsterLevelDropdown() const
{
    auto monsterLevels = QFile(":/res/em_level.json");
    if (!monsterLevels.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open monster_level.json");
    }

    QJsonArray levels = QJsonDocument::fromJson(monsterLevels.readAll()).array();
    for (const auto level : levels)
    {
        const auto obj = level.toObject();
        ui.comboMonsterLevel->addItem(obj["Name"].toString(), obj["Value"].toInt());
    }
}

void MHGUQuestEditor::initMapDropdown() const
{
    auto mapsFile = QFile(":/res/map_names.json");
    if (!mapsFile.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open map_names.json");
    }

    QJsonArray maps = QJsonDocument::fromJson(mapsFile.readAll()).array();
    for (const auto map : maps)
    {
        const auto obj = map.toObject();
        ui.comboMap->addItem(obj["Name"].toString(), obj["Value"].toInt());
    }

    ui.comboMap->model()->sort(0);
}

void MHGUQuestEditor::initSpawnTypeDropdown() const {
    auto spawnTypes = QFile(":/res/quest_start_type.json");
    if (!spawnTypes.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open spawn_types.json");
    }

    QJsonArray types = QJsonDocument::fromJson(spawnTypes.readAll()).array();
    for (const auto [i, type] : std::views::enumerate(types))
    {
        ui.comboSpawnType->addItem(type.toString(), i);
    }
}

void MHGUQuestEditor::initBgmDropdown() const {
    auto questBgm = QFile(":/res/quest_bgm.json");
    if (!questBgm.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open bgm.json");
    }

    QJsonArray bgms = QJsonDocument::fromJson(questBgm.readAll()).array();
    for (const auto [i, bgm] : std::views::enumerate(bgms))
    {
        ui.comboBgm->addItem(bgm.toString(), i);
    }
}

void MHGUQuestEditor::initRequirementsDropdowns() const {
    auto questRequirements = QFile(":/res/quest_requirement.json");
    if (!questRequirements.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open quest_requirement.json");
    }

    QJsonArray requirements = QJsonDocument::fromJson(questRequirements.readAll()).array();
    for (const auto [i, requirement] : std::views::enumerate(requirements))
    {
        ui.comboRequirement1->addItem(requirement.toString(), i);
        ui.comboRequirement2->addItem(requirement.toString(), i);
        ui.comboComboRequirement->addItem(requirement.toString(), i);
    }
}

void MHGUQuestEditor::initClearTypeDropdown() const {
    auto questClearTypes = QFile(":/res/quest_clear_type.json");
    if (!questClearTypes.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open quest_clear_type.json");
    }

    QJsonArray clearTypes = QJsonDocument::fromJson(questClearTypes.readAll()).array();
    for (const auto [i, clearType] : std::views::enumerate(clearTypes))
    {
        ui.comboClearType->addItem(clearType.toString(), i);
    }
}

void MHGUQuestEditor::initObjectiveDropdowns() const
{
    auto questObjectives = QFile(":/res/quest_objective.json");
    if (!questObjectives.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open quest_objective.json");
    }

    QJsonArray objectives = QJsonDocument::fromJson(questObjectives.readAll()).array();
    for (const auto [i, objective] : std::views::enumerate(objectives))
    {
        ui.comboObjectiveType1->addItem(objective.toString(), i);
        ui.comboObjectiveType2->addItem(objective.toString(), i);
        ui.comboSubObjectiveType->addItem(objective.toString(), i);
    }

    const auto indexChanged = [this](int index, QComboBox* idCombo) {
        const auto type = (Resources::QuestClearParam)index;
        idCombo->clear();
        switch (type) {
        case Resources::QuestClearParam::Hunt: [[fallthrough]];
        case Resources::QuestClearParam::Capture: [[fallthrough]];
        case Resources::QuestClearParam::HuntAllLargeMonsters: [[fallthrough]];
        case Resources::QuestClearParam::SlayTotalOfTargets:
            idCombo->setModel(nullptr);
            for (auto i = 0; i < ui.comboMonster1->count(); ++i) {
                idCombo->addItem(ui.comboMonster1->itemText(i), ui.comboMonster1->itemData(i));
            }
            break;
        case Resources::QuestClearParam::DeliverItem:
            idCombo->setModel(itemNamesModel);
            break;
        case Resources::QuestClearParam::None: [[fallthrough]];
        case Resources::QuestClearParam::EarnWycademyPoints: [[fallthrough]];
        case Resources::QuestClearParam::DeliverPawPass: [[fallthrough]];
        case Resources::QuestClearParam::BreakParts: [[fallthrough]];
        case Resources::QuestClearParam::MountAndTopple:
            break;
        }
    };

    using namespace std::placeholders;

    connect(ui.comboObjectiveType1, &QComboBox::currentIndexChanged, this, [indexChanged, this] (int index) {
        indexChanged(index, ui.comboObjectiveId1);
    });
    connect(ui.comboObjectiveType2, &QComboBox::currentIndexChanged, this, [indexChanged, this](int index) {
        indexChanged(index, ui.comboObjectiveId2);
    });
    connect(ui.comboSubObjectiveType, &QComboBox::currentIndexChanged, this, [indexChanged, this](int index) {
        indexChanged(index, ui.comboSubObjectiveId);
    });
}

void MHGUQuestEditor::initItemLevelDropdowns() const
{
    auto carveLevels = QFile(":/res/quest_carve_level.json");
    if (!carveLevels.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open quest_carve_level.json");
    }

    auto levels = QJsonDocument::fromJson(carveLevels.readAll()).array();
    for (const auto [i, level] : std::views::enumerate(levels))
    {
        ui.comboCarveLevel->addItem(level.toString(), i);
    }

    auto gatheringLevels = QFile(":/res/quest_gather_level.json");
    if (!gatheringLevels.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open quest_gather_level.json");
    }

    levels = QJsonDocument::fromJson(gatheringLevels.readAll()).array();
    for (const auto [i, level] : std::views::enumerate(levels))
    {
        ui.comboGatheringLevel->addItem(level.toString(), i);
    }

    auto fishingLevels = QFile(":/res/quest_fishing_level.json");
    if (!fishingLevels.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open quest_fishing_level.json");
    }

    levels = QJsonDocument::fromJson(fishingLevels.readAll()).array();
    for (const auto [i, level] : std::views::enumerate(levels))
    {
        ui.comboFishingLevel->addItem(level.toString(), i);
    }
}

void MHGUQuestEditor::initItemNames()
{
    auto itemNamesFile = QFile(":/res/item_names.json");
    if (!itemNamesFile.open(QIODevice::ReadOnly))
    {
        qFatal("Failed to open item_names.json");
    }

    const auto names = QJsonDocument::fromJson(itemNamesFile.readAll());
    itemNames = names.toVariant().toStringList();
    itemNamesModel = new QStringListModel(itemNames, this);
}

void MHGUQuestEditor::addRemEntry(const QString& remName, s32 tabIndex)
{
    const auto tab = ui.tabWidgetRewards->widget(tabIndex);
    const auto scrollArea = tab->findChild<QScrollArea*>(QStringLiteral("scrollAreaRem%1").arg(remName));
    const auto layout = (QVBoxLayout*)scrollArea->widget()->layout();

    const auto deleteButton = new QToolButton;
    const auto itemCombo = new QComboBox;
    const auto amountBox = new QSpinBox;
    const auto chanceBox = new QSpinBox;
    const auto itemLayout = new QHBoxLayout;

    itemCombo->setModel(itemNamesModel);
    chanceBox->setSuffix("%");

    deleteButton->setText("⨯");
    connect(deleteButton, &QToolButton::clicked, [itemLayout] {
        itemLayout->itemAt(0)->widget()->deleteLater(); // Delete button
        itemLayout->itemAt(1)->widget()->deleteLater(); // Item combo
        itemLayout->itemAt(2)->widget()->deleteLater(); // Amount box
        itemLayout->itemAt(3)->widget()->deleteLater(); // Chance box
        itemLayout->deleteLater();
    });

    itemCombo->setCurrentIndex(0);
    amountBox->setValue(0);
    chanceBox->setValue(0);

    itemLayout->addWidget(deleteButton, 0);
    itemLayout->addWidget(itemCombo, 2);
    itemLayout->addWidget(amountBox, 1);
    itemLayout->addWidget(chanceBox, 1);
    layout->addLayout(itemLayout);
}

void MHGUQuestEditor::loadSettings()
{
    QSettings settings("Fexty", "MHGU Quest Editor");

    settings.beginGroup("MainWindow");

    const auto geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty())
        restoreGeometry(geometry);

    settings.endGroup();
    settings.beginGroup("QuestEditor");

    questListPath = settings.value("quest_list_path").toString();
    autoUpdateQuestList = settings.value("auto_update_quest_list").toBool();
    recentFiles = settings.value("recent_files").toStringList();

    settings.endGroup();
}

void MHGUQuestEditor::saveSettings() const
{
    QSettings settings("Fexty", "MHGU Quest Editor");

    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();

    settings.beginGroup("QuestEditor");
    settings.setValue("quest_list_path", questListPath);
    settings.setValue("auto_update_quest_list", autoUpdateQuestList);
    settings.setValue("recent_files", recentFiles);
    settings.endGroup();
}

void MHGUQuestEditor::onOpenFile()
{
    // Open file dialog
    const auto path = QFileDialog::getOpenFileName(
        this, 
        "Open Quest File", 
        {}, 
        "Archive Files (*.arc);;Quest Files (*.mib *.ext)"
    );

    if (path.isEmpty())
        return;

    loadFile(path);
}

void MHGUQuestEditor::onSaveFile()
{
    if (arc) 
    {
        saveQuestArc();
    }
    else 
    {
        saveQuestFile();
    }
}

void MHGUQuestEditor::onSaveFileAs()
{
    const auto path = QFileDialog::getSaveFileName(
        this,
        "Save Quest File",
        {},
        "Archive Files (*.arc);;Quest Files (*.mib *.ext)"
    );

    if (path.isEmpty())
        return;

    if (arc)
    {
        saveQuestArc(path);
    }
    else
    {
        saveQuestFile(path);
    }
}

void MHGUQuestEditor::loadFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical("Failed to open file %s", qUtf8Printable(path));
        return;
    }

    if (std::ranges::find(recentFiles, path) == recentFiles.end())
    {
        recentFiles.push_front(path);
        if (recentFiles.size() > 10)
            recentFiles.pop_back();
    }
    else
    {
        recentFiles.removeOne(path);
        recentFiles.push_front(path);
    }

    if (path.endsWith(".mib") || path.endsWith(".ext"))
    {
        arc.reset();
        questData = Resources::QuestData::deserialize(file.readAll());

        loadQuestDataIntoUi();
    }
    else if (path.endsWith(".arc"))
    {
        constexpr auto isQuestArc = [](const std::filesystem::path& p) {
            const auto filename = p.stem().string();
            if (filename.size() != 8)
                return false;

            if (filename[0] != 'q')
                return false;

            return std::all_of(filename.begin() + 1, filename.end(), isdigit);
        };

        const std::filesystem::path fsPath = path.toStdWString();
        if (!isQuestArc(fsPath))
        {
            qCritical("Invalid quest arc %s", qUtf8Printable(path));
            return;
        }

        arc = std::make_unique<Resources::QuestArc>(fsPath);

        loadQuestArc();
    }
    else
    {
        qCritical("Invalid file extension %s", qUtf8Printable(path));
        return;
    }

    openedFile = path;
    ui.tabWidgetLanguage->setCurrentIndex(0);
    ui.tabWidgetRoot->setCurrentIndex(0);
    ui.tabWidgetQuestData->setCurrentIndex(0);
}

void MHGUQuestEditor::loadQuestArc()
{
    using namespace Resources;

    gmds.clear();
    rems.clear();

    questData = Resources::QuestData::deserialize(arc->getQuestData().getData());

    constexpr auto setText = [](QLineEdit* name, QLineEdit* client, 
        QPlainTextEdit* desc, QPlainTextEdit* zako, 
        QPlainTextEdit* obj, QPlainTextEdit* failure, 
        QLineEdit* sub, const Resources::Gmd& gmd) {
        name->setText(gmd.Entries[0].c_str());
        client->setText(gmd.Entries[1].c_str());
        desc->setPlainText(gmd.Entries[2].c_str());
        zako->setPlainText(gmd.Entries[3].c_str());
        obj->setPlainText(gmd.Entries[4].c_str());
        failure->setPlainText(gmd.Entries[5].c_str());
        sub->setText(gmd.Entries[6].c_str());
    };

    ui.tabWidgetRoot->setTabEnabled(1, true); // Enable quest info tab

    for (s32 language = Language::Eng; language < Language::Count; ++language)
    {
        const auto gmdEntry = arc->getGmd(language, questData.Info[language].File);
        if (!gmdEntry)
        {
            ui.tabWidgetLanguage->setTabEnabled(language, false);
            gmds.emplace_back();
            qCritical("Failed to find GMD for language %d", language);
            continue;
        }

        ui.tabWidgetLanguage->setTabEnabled(language, true);
        const auto& gmd = gmds.emplace_back(Gmd::deserialize(gmdEntry->getData()));

        switch (language)
        {
        case Language::Eng:
            setText(
                ui.textNameEng, ui.textClientEng,
                ui.textDescriptionEng, ui.textZakoEng,
                ui.textObjectiveEng, ui.textFailureEng,
                ui.textSubquestEng, gmd
            );
            break;
        case Language::Fre:
            setText(
                ui.textNameFre, ui.textClientFre,
                ui.textDescriptionFre, ui.textZakoFre,
                ui.textObjectiveFre, ui.textFailureFre,
                ui.textSubquestFre, gmd
            );
            break;
        case Language::Ger:
            setText(
                ui.textNameGer, ui.textClientGer,
                ui.textDescriptionGer, ui.textZakoGer,
                ui.textObjectiveGer, ui.textFailureGer,
                ui.textSubquestGer, gmd
            );
            break;
        case Language::Ita:
            setText(
                ui.textNameIta, ui.textClientIta,
                ui.textDescriptionIta, ui.textZakoIta,
                ui.textObjectiveIta, ui.textFailureIta,
                ui.textSubquestIta, gmd
            );
            break;
        case Language::Spa:
            setText(
                ui.textNameSpa, ui.textClientSpa,
                ui.textDescriptionSpa, ui.textZakoSpa,
                ui.textObjectiveSpa, ui.textFailureSpa,
                ui.textSubquestSpa, gmd
            );
            break;
        case Language::ChT:
            setText(
                ui.textNameChT, ui.textClientChT,
                ui.textDescriptionChT, ui.textZakoChT,
                ui.textObjectiveChT, ui.textFailureChT,
                ui.textSubquestChT, gmd
            );
            break;
        case Language::ChS:
            setText(
                ui.textNameChS, ui.textClientChS,
                ui.textDescriptionChS, ui.textZakoChS,
                ui.textObjectiveChS, ui.textFailureChS,
                ui.textSubquestChS, gmd
            );
            break;
        case Language::Count: [[fallthrough]];
        default:
            break;
        }
    }

    questLink = std::make_unique<QuestLink>(QuestLink::deserialize(arc->getQuestLink().getData()));
    const auto resources = questLink->resolve(*arc);

    ui.tabWidgetRoot->setTabEnabled(2, true); // Enable rewards tab

    rems.emplace_back(resources.RemMain[0] ? Rem::deserialize(resources.RemMain[0]->getData()) : Resources::Rem{});
    rems.emplace_back(resources.RemMain[1] ? Rem::deserialize(resources.RemMain[1]->getData()) : Resources::Rem{});
    rems.emplace_back(resources.RemAdd[0] ? Rem::deserialize(resources.RemAdd[0]->getData()) : Resources::Rem{});
    rems.emplace_back(resources.RemAdd[1] ? Rem::deserialize(resources.RemAdd[1]->getData()) : Resources::Rem{});
    rems.emplace_back(resources.RemSub ? Rem::deserialize(resources.RemSub->getData()) : Resources::Rem{});

    loadRemIntoUi(rems[0], "MainA", 0);
    loadRemIntoUi(rems[1], "MainB", 1);
    loadRemIntoUi(rems[2], "ExtraA", 2);
    loadRemIntoUi(rems[3], "ExtraB", 3);
    loadRemIntoUi(rems[4], "Sub", 4);

    ui.tabWidgetRoot->setTabEnabled(3, true); // Enable small monsters tab

    for (s32 i = 0; i < 3; ++i)
    {
        if (resources.EmSetList[i])
        {
            emSetListEditors[i]->setEsl(EmSetList::deserialize(resources.EmSetList[i]->getData()));
        }
    }

    loadQuestDataIntoUi();
}

void MHGUQuestEditor::saveQuestArc(const QString& path)
{
    if (!arc)
    {
        qCritical("No quest arc loaded");
        return;
    }

    // Save UI
    saveQuestDataFromUi();
    saveQuestInfoFromUi();
    saveRemFromUi(rems[0], "MainA", 0);
    saveRemFromUi(rems[1], "MainB", 1);
    saveRemFromUi(rems[2], "ExtraA", 2);
    saveRemFromUi(rems[3], "ExtraB", 3);
    saveRemFromUi(rems[4], "Sub", 4);

    // Save quest info
    using Resources::Language;
    using namespace Qt::StringLiterals;

    for (s32 language = Language::Eng; language < Language::Count; ++language)
    {
        if (language >= gmds.size() || !ui.tabWidgetLanguage->isTabEnabled(language))
            continue;

        const auto& gmd = gmds[language];
        const auto gmdEntry = arc->getGmd(language, questData.Info[language].File);
        (void)std::snprintf(
            questData.Info[language].File, 
            16, 
            "%07d_%s", 
            questData.Id, 
            Language::toString(language).toStdString().c_str()
        );

        const auto serializedGmd = Resources::Gmd::serialize(gmd);
        gmdEntry->Path = QStringLiteral(R"(%1\quest\questData\questData_%2)")
            .arg(Language::toString(language))
            .arg(questData.Info[language].File);
        gmdEntry->setData({
            (const u8*)serializedGmd.data(), 
            (size_t)serializedGmd.size()
        });
    }

    // Save quest data
    const auto serialized = Resources::QuestData::serialize(questData);
    auto& questDataEntry = arc->getQuestData();
    questDataEntry.Path = QStringLiteral(R"(loc\quest\questData\questData_%1)").arg(questData.Id, 7, 10, QChar(u8'0'));
    questDataEntry.setData({
        (const u8*)serialized.data(),
        (size_t)serialized.size()
    });

    const auto resources = questLink->resolve(*arc);

    // Save rem
    const auto saveRem = [this](const Resources::Rem& rem, Resources::ArcEntry* entry, Resources::LinkResource& resource) {
        if (rem.size() == 0)
        {
            questLink->clearResource(resource);
        }
        else
        {
            // Entries were added to an empty resource, so we need to create a new entry
            if (!entry && Resources::QuestLink::isEmptyResource(resource))
            {
                // Generate a random 6 digit number for the id
                // Highest vanilla ids end at around 730000, so we'll start at 750000
                // Chance of collision is low enough to ignore
                const auto id = QRandomGenerator::global()->bounded(750000, 999999);
                arc->addRem(id, Resources::Rem::serialize(rem));
                questLink->setRemResource(resource, id);
            }
            else 
            {
                entry->setData(Resources::Rem::serialize(rem));
            }
        }
    };

    saveRem(rems[0], resources.RemMain[0], questLink->RemMain[0]);
    saveRem(rems[1], resources.RemMain[1], questLink->RemMain[1]);
    saveRem(rems[2], resources.RemAdd[0], questLink->RemAdd[0]);
    saveRem(rems[3], resources.RemAdd[1], questLink->RemAdd[1]);
    saveRem(rems[4], resources.RemSub, questLink->RemSub);

    // Save esl
    const auto saveEsl = [this](const EmSetListEditor* editor, Resources::ArcEntry* entry, Resources::LinkResource& resource) {
        if (editor->getEsl().empty())
        {
            questLink->clearResource(resource);
        }
        else
        {
            // Entries were added to an empty resource, so we need to create a new entry
            if (!entry && Resources::QuestLink::isEmptyResource(resource))
            {
                // Generate a random 3 digit number for the id
                // Vanilla ids never exceed 100 so we'll start there
                const auto esl = Resources::EmSetList::serialize(editor->getEsl());
                int id;
                do {
                    id = QRandomGenerator::global()->bounded(100, 999);
                } while (!arc->addEsl((u32)questData.Map, id, esl));
                questLink->setEslResource(resource, (u32)questData.Map, id);
            }
            else
            {
                entry->setData(Resources::EmSetList::serialize(editor->getEsl()));
            }
        }
    };

    saveEsl(emSetListEditors[0], resources.EmSetList[0], questLink->EmSetList[0]);
    saveEsl(emSetListEditors[1], resources.EmSetList[1], questLink->EmSetList[1]);
    saveEsl(emSetListEditors[2], resources.EmSetList[2], questLink->EmSetList[2]);

    if (path.isEmpty())
        arc->save();
    else
        arc->save(path.toStdWString());

    if (autoUpdateQuestList)
        saveQuestArcToQuestList();
}

void MHGUQuestEditor::saveQuestFile(const QString& path) const
{
    QFile file(path.isEmpty() ? openedFile : path);
    file.write(Resources::QuestData::serialize(questData));
}

void MHGUQuestEditor::saveQuestArcToQuestList() const
{
    if (!arc)
    {
        qCritical("No quest arc loaded");
        return;
    }

    if (questListPath.isEmpty())
    {
        qCritical("No quest list path set");
        return;
    }

    if (!QFile::exists(questListPath))
    {
        qCritical("Quest list path does not exist");
        return;
    }

    const auto& serialized = arc->getQuestData();
    Resources::QuestArc questList(questListPath.toStdWString());
    const auto questDataEntry = questList.getQuestData(questData.Id);
    if (questDataEntry) // Update existing entry
    {
        questDataEntry->setData(serialized.getData(false), false);
        questDataEntry->RealSize = serialized.RealSize; // Update real size since setData(..., false) doesn't update it
    }
    else // Insert new entry
    {
        questList.addQuestData(questData.Id, serialized.getData(false), true, serialized.RealSize);
    }

    for (s32 language = 0; language < gmds.size(); ++language)
    {
        if (language >= gmds.size() || !ui.tabWidgetLanguage->isTabEnabled(language))
            continue;

        const auto name = questData.Info[language].File;
        const auto gmdEntry = arc->getGmd(language, name);
        const auto targetGmd = questList.getGmd(language, name);
        if (targetGmd) // Update existing entry
        {
            targetGmd->setData(gmdEntry->getData(false), false);
            targetGmd->RealSize = gmdEntry->RealSize; // Update real size since setData(..., false) doesn't update it
        }
        else // Insert new entry
        {
            questList.addGmd(language, name, gmdEntry->getData(false), true, gmdEntry->RealSize);
        }
    }

    questList.save();
}

void MHGUQuestEditor::loadQuestDataIntoUi()
{
    const auto setIndexFromDataIntegral = []<Integral T> (QComboBox * combo, T data) {
        for (auto i = 0; i < combo->count(); ++i)
        {
            if (combo->itemData(i).toInt() == data)
            {
                combo->setCurrentIndex(i);
                return;
            }
        }
    };
    const auto setIndexFromData = [this, setIndexFromDataIntegral]<AnyIntegral T> (QComboBox * combo, T data) {
        if constexpr (Integral<T>)
        {
            setIndexFromDataIntegral(combo, data);
        } else
        {
            setIndexFromDataIntegral(combo, static_cast<std::underlying_type_t<T>>(data));
        }
    };
    const auto setIndex = [this]<AnyIntegral T> (QComboBox * combo, T index) {
        if constexpr (Integral<T>)
        {
            combo->setCurrentIndex(index);
        } else
        {
            combo->setCurrentIndex(static_cast<std::underlying_type_t<T>>(index));
        }
    };

    // General
    ui.spinBoxQuestId->setValue(questData.Id);
    setIndexFromData(ui.comboQuestType, questData.Type);
    setIndexFromData(ui.comboQuestSubType, questData.SubType);
    setIndexFromData(ui.comboQuestLevel, questData.Level);
    setIndexFromData(ui.comboMonsterLevel, questData.EnemyLevel);
    setIndexFromData(ui.comboMap, questData.Map);
    setIndexFromData(ui.comboSpawnType, questData.StartType);
    ui.spinBoxTimeLimit->setValue(questData.QuestTime);
    ui.spinBoxFaints->setValue(questData.Faints);
    ui.spinBoxArenaEquipId->setValue(questData.ArenaEquipId);
    setIndexFromData(ui.comboBgm, questData.BgmType);
    setIndexFromData(ui.comboRequirement1, questData.Requirement1);
    setIndexFromData(ui.comboRequirement2, questData.Requirement2);
    setIndexFromData(ui.comboComboRequirement, questData.ComboRequirement);
    setIndexFromData(ui.comboClearType, questData.ClearType);
    ui.spinBoxFaints->setValue(questData.Faints);
    ui.spinBoxQuestZenny->setValue(questData.Reward);
    ui.spinBoxSubquestZenny->setValue(questData.SubReward);
    ui.spinBoxQuestPoints->setValue(questData.ClearVillagePoints);
    ui.spinBoxSubquestPoints->setValue(questData.SubVillagePoints);
    ui.spinBoxQuestHrp->setValue(questData.HunterRankPoints);
    ui.spinBoxSubquestHrp->setValue(questData.SubHunterRankPoints);

    setIndex(ui.comboIcon1, questData.Icons[0]);
    setIndex(ui.comboIcon2, questData.Icons[1]);
    setIndex(ui.comboIcon3, questData.Icons[2]);
    setIndex(ui.comboIcon4, questData.Icons[3]);
    setIndex(ui.comboIcon5, questData.Icons[4]);

    // Objectives
    const auto setIndexFromDataObjective = [this, setIndexFromData](QComboBox* combo, Resources::QuestClearParam param, u16 value) {
        switch (param) {
        case Resources::QuestClearParam::None: [[fallthrough]];
        case Resources::QuestClearParam::EarnWycademyPoints: [[fallthrough]];
        case Resources::QuestClearParam::DeliverPawPass: [[fallthrough]];
        case Resources::QuestClearParam::BreakParts: [[fallthrough]];
        case Resources::QuestClearParam::MountAndTopple:
            break;
        case Resources::QuestClearParam::Hunt: [[fallthrough]];
        case Resources::QuestClearParam::Capture: [[fallthrough]];
        case Resources::QuestClearParam::HuntAllLargeMonsters: [[fallthrough]];
        case Resources::QuestClearParam::SlayTotalOfTargets:
            setIndexFromData(combo, value);
            break;
        case Resources::QuestClearParam::DeliverItem:
            combo->setCurrentIndex(value);
            break;
        }
    };

    setIndexFromData(ui.comboObjectiveType1, questData.ClearConditions[0].Param);
    setIndexFromData(ui.comboObjectiveType2, questData.ClearConditions[1].Param);
    setIndexFromData(ui.comboSubObjectiveType, questData.SubClearCondition.Param);

    setIndexFromDataObjective(ui.comboObjectiveId1, questData.ClearConditions[0].Param, questData.ClearConditions[0].Value);
    setIndexFromDataObjective(ui.comboObjectiveId2, questData.ClearConditions[1].Param, questData.ClearConditions[1].Value);
    setIndexFromDataObjective(ui.comboSubObjectiveId, questData.SubClearCondition.Param, questData.SubClearCondition.Value);

    ui.spinBoxObjective1Amount->setValue(questData.ClearConditions[0].Count);
    ui.spinBoxObjective2Amount->setValue(questData.ClearConditions[1].Count);
    ui.spinBoxSubObjectiveAmount->setValue(questData.SubClearCondition.Count);

    // Monsters
    setIndexFromData(ui.comboMonster1, questData.Monsters[0].Id);
    setIndexFromData(ui.comboMonster2, questData.Monsters[1].Id);
    setIndexFromData(ui.comboMonster3, questData.Monsters[2].Id);
    setIndexFromData(ui.comboMonster4, questData.Monsters[3].Id);
    setIndexFromData(ui.comboMonster5, questData.Monsters[4].Id);

    ui.spinBoxMonster1Special->setValue(questData.Monsters[0].SubType);
    ui.spinBoxMonster2Special->setValue(questData.Monsters[1].SubType);
    ui.spinBoxMonster3Special->setValue(questData.Monsters[2].SubType);
    ui.spinBoxMonster4Special->setValue(questData.Monsters[3].SubType);
    ui.spinBoxMonster5Special->setValue(questData.Monsters[4].SubType);

    ui.spinBoxMonster1State->setValue(questData.Monsters[0].AuraType);
    ui.spinBoxMonster2State->setValue(questData.Monsters[1].AuraType);
    ui.spinBoxMonster3State->setValue(questData.Monsters[2].AuraType);
    ui.spinBoxMonster4State->setValue(questData.Monsters[3].AuraType);
    ui.spinBoxMonster5State->setValue(questData.Monsters[4].AuraType);

    ui.spinBoxMonster1RestoreAmount->setValue(questData.Monsters[0].RestoreAmount);
    ui.spinBoxMonster2RestoreAmount->setValue(questData.Monsters[1].RestoreAmount);
    ui.spinBoxMonster3RestoreAmount->setValue(questData.Monsters[2].RestoreAmount);
    ui.spinBoxMonster4RestoreAmount->setValue(questData.Monsters[3].RestoreAmount);
    ui.spinBoxMonster5RestoreAmount->setValue(questData.Monsters[4].RestoreAmount);

    setIndex(ui.comboMonster1Health, questData.Monsters[0].HealthTableIndex);
    setIndex(ui.comboMonster2Health, questData.Monsters[1].HealthTableIndex);
    setIndex(ui.comboMonster3Health, questData.Monsters[2].HealthTableIndex);
    setIndex(ui.comboMonster4Health, questData.Monsters[3].HealthTableIndex);
    setIndex(ui.comboMonster5Health, questData.Monsters[4].HealthTableIndex);

    setIndex(ui.comboMonster1Attack, questData.Monsters[0].AttackTableIndex);
    setIndex(ui.comboMonster2Attack, questData.Monsters[1].AttackTableIndex);
    setIndex(ui.comboMonster3Attack, questData.Monsters[2].AttackTableIndex);
    setIndex(ui.comboMonster4Attack, questData.Monsters[3].AttackTableIndex);
    setIndex(ui.comboMonster5Attack, questData.Monsters[4].AttackTableIndex);

    setIndex(ui.comboMonster1Other, questData.Monsters[0].HealthTableIndex);
    setIndex(ui.comboMonster2Other, questData.Monsters[1].HealthTableIndex);
    setIndex(ui.comboMonster3Other, questData.Monsters[2].HealthTableIndex);
    setIndex(ui.comboMonster4Other, questData.Monsters[3].HealthTableIndex);
    setIndex(ui.comboMonster5Other, questData.Monsters[4].HealthTableIndex);

    ui.spinBoxMonster1Difficulty->setValue(questData.Monsters[0].Difficulty);
    ui.spinBoxMonster2Difficulty->setValue(questData.Monsters[1].Difficulty);
    ui.spinBoxMonster3Difficulty->setValue(questData.Monsters[2].Difficulty);
    ui.spinBoxMonster4Difficulty->setValue(questData.Monsters[3].Difficulty);
    ui.spinBoxMonster5Difficulty->setValue(questData.Monsters[4].Difficulty);

    ui.spinBoxMonster1Size->setValue(questData.Monsters[0].Size);
    ui.spinBoxMonster2Size->setValue(questData.Monsters[1].Size);
    ui.spinBoxMonster3Size->setValue(questData.Monsters[2].Size);
    ui.spinBoxMonster4Size->setValue(questData.Monsters[3].Size);
    ui.spinBoxMonster5Size->setValue(questData.Monsters[4].Size);

    ui.spinBoxMonster1SizeTable->setValue(questData.Monsters[0].SizeTableIndex);
    ui.spinBoxMonster2SizeTable->setValue(questData.Monsters[1].SizeTableIndex);
    ui.spinBoxMonster3SizeTable->setValue(questData.Monsters[2].SizeTableIndex);
    ui.spinBoxMonster4SizeTable->setValue(questData.Monsters[3].SizeTableIndex);
    ui.spinBoxMonster5SizeTable->setValue(questData.Monsters[4].SizeTableIndex);

    ui.spinBoxMonster1StaminaTable->setValue(questData.Monsters[0].StaminaTableIndex);
    ui.spinBoxMonster2StaminaTable->setValue(questData.Monsters[1].StaminaTableIndex);
    ui.spinBoxMonster3StaminaTable->setValue(questData.Monsters[2].StaminaTableIndex);
    ui.spinBoxMonster4StaminaTable->setValue(questData.Monsters[3].StaminaTableIndex);
    ui.spinBoxMonster5StaminaTable->setValue(questData.Monsters[4].StaminaTableIndex);

    setIndex(ui.comboZakoHealth, questData.SmallMonsterHpIndex);
    setIndex(ui.comboZakoAttack, questData.SmallMonsterAtkIndex);
    setIndex(ui.comboZakoOther, questData.SmallMonsterOtherIndex);

    // Items
    ui.spinBoxSuppliesALabel->setValue(questData.Supplies[0].SuppLabel);
    ui.spinBoxSuppliesAType->setValue(questData.Supplies[0].SuppType);
    ui.spinBoxSuppliesATarget->setValue(questData.Supplies[0].SuppTarget);
    ui.spinBoxSuppliesATargetAmount->setValue(questData.Supplies[0].SuppTargetCount);

    ui.spinBoxSuppliesBLabel->setValue(questData.Supplies[1].SuppLabel);
    ui.spinBoxSuppliesBType->setValue(questData.Supplies[1].SuppType);
    ui.spinBoxSuppliesBTarget->setValue(questData.Supplies[1].SuppTarget);
    ui.spinBoxSuppliesBTargetAmount->setValue(questData.Supplies[1].SuppTargetCount);

    setIndexFromData(ui.comboCarveLevel, questData.CarveLevel);
    setIndexFromData(ui.comboGatheringLevel, questData.GatheringLevel);
    setIndexFromData(ui.comboFishingLevel, questData.FishingLevel);

    ui.spinBoxRewardBoxes1->setValue(questData.RemAddFrame[0]);
    ui.spinBoxRewardBoxes2->setValue(questData.RemAddFrame[1]);
    ui.spinBoxMaxExtraBoxes->setValue(questData.RemAddLotMax);

    // Spawns
    ui.spinBoxMonster2SetType->setValue(questData.EnemySet2.SetType);
    ui.spinBoxMonster2SetTargetId->setValue(questData.EnemySet2.TargetId);
    ui.spinBoxMonster2SetTargetAmount->setValue(questData.EnemySet2.TargetCount);

    ui.spinBoxMonster3SetType->setValue(questData.EnemySet3.SetType);
    ui.spinBoxMonster3SetTargetId->setValue(questData.EnemySet3.TargetId);
    ui.spinBoxMonster3SetTargetAmount->setValue(questData.EnemySet3.TargetCount);

    ui.spinBoxMonster1SpawnType->setValue(questData.MonsterSpawns[0].SpawnType);
    ui.spinBoxMonster2SpawnType->setValue(questData.MonsterSpawns[1].SpawnType);
    ui.spinBoxMonster3SpawnType->setValue(questData.MonsterSpawns[2].SpawnType);
    ui.spinBoxMonster4SpawnType->setValue(questData.MonsterSpawns[3].SpawnType);
    ui.spinBoxMonster5SpawnType->setValue(questData.MonsterSpawns[4].SpawnType);

    ui.spinBoxMonster1SpawnTargetType->setValue(questData.MonsterSpawns[0].SpawnTargetType);
    ui.spinBoxMonster2SpawnTargetType->setValue(questData.MonsterSpawns[1].SpawnTargetType);
    ui.spinBoxMonster3SpawnTargetType->setValue(questData.MonsterSpawns[2].SpawnTargetType);
    ui.spinBoxMonster4SpawnTargetType->setValue(questData.MonsterSpawns[3].SpawnTargetType);
    ui.spinBoxMonster5SpawnTargetType->setValue(questData.MonsterSpawns[4].SpawnTargetType);

    ui.spinBoxMonster1SpawnTargetAmount->setValue(questData.MonsterSpawns[0].SpawnTargetCount);
    ui.spinBoxMonster2SpawnTargetAmount->setValue(questData.MonsterSpawns[1].SpawnTargetCount);
    ui.spinBoxMonster3SpawnTargetAmount->setValue(questData.MonsterSpawns[2].SpawnTargetCount);
    ui.spinBoxMonster4SpawnTargetAmount->setValue(questData.MonsterSpawns[3].SpawnTargetCount);
    ui.spinBoxMonster5SpawnTargetAmount->setValue(questData.MonsterSpawns[4].SpawnTargetCount);

    ui.spinBoxStrayRandom->setValue(questData.StrayRand);
    ui.spinBoxStrayStart->setValue(questData.StrayStartTime);
    ui.spinBoxStrayStartRandom->setValue(questData.StrayStartRand);
    ui.spinBoxStrayLimit3->setValue(questData.StrayLimit345[0]);
    ui.spinBoxStrayLimit4->setValue(questData.StrayLimit345[1]);
    ui.spinBoxStrayLimit5->setValue(questData.StrayLimit345[2]);
    ui.spinBoxStrayRandom3->setValue(questData.StrayRand345[0]);
    ui.spinBoxStrayRandom4->setValue(questData.StrayRand345[1]);
    ui.spinBoxStrayRandom5->setValue(questData.StrayRand345[2]);
    ui.spinBoxExtraTicketCount->setValue(questData.ExtraTicketCount);
}

void MHGUQuestEditor::saveQuestDataFromUi()
{
    const auto setFieldFromData = []<AnyIntegral T>(const QComboBox* combo, T& data) {
        data = static_cast<T>(combo->currentData().toInt());
    };

    const auto setFieldFromIndex = []<AnyIntegral T>(const QComboBox* combo, T& data) {
        data = static_cast<T>(combo->currentIndex());
    };

    // General
    questData.Id = ui.spinBoxQuestId->value();
    setFieldFromData(ui.comboQuestType, questData.Type);
    setFieldFromData(ui.comboQuestSubType, questData.SubType);
    setFieldFromData(ui.comboQuestLevel, questData.Level);
    setFieldFromData(ui.comboMonsterLevel, questData.EnemyLevel);
    setFieldFromData(ui.comboMap, questData.Map);
    setFieldFromData(ui.comboSpawnType, questData.StartType);
    questData.QuestTime = ui.spinBoxTimeLimit->value();
    questData.Faints = ui.spinBoxFaints->value();
    questData.ArenaEquipId = ui.spinBoxArenaEquipId->value();
    setFieldFromData(ui.comboBgm, questData.BgmType);
    setFieldFromData(ui.comboRequirement1, questData.Requirement1);
    setFieldFromData(ui.comboRequirement2, questData.Requirement2);
    setFieldFromData(ui.comboComboRequirement, questData.ComboRequirement);
    setFieldFromData(ui.comboClearType, questData.ClearType);
    questData.Reward = ui.spinBoxQuestZenny->value();
    questData.SubReward = ui.spinBoxSubquestZenny->value();
    questData.ClearVillagePoints = ui.spinBoxQuestPoints->value();
    questData.SubVillagePoints = ui.spinBoxSubquestPoints->value();
    questData.HunterRankPoints = ui.spinBoxQuestHrp->value();
    questData.SubHunterRankPoints = ui.spinBoxSubquestHrp->value();

    setFieldFromIndex(ui.comboIcon1, questData.Icons[0]);
    setFieldFromIndex(ui.comboIcon2, questData.Icons[1]);
    setFieldFromIndex(ui.comboIcon3, questData.Icons[2]);
    setFieldFromIndex(ui.comboIcon4, questData.Icons[3]);
    setFieldFromIndex(ui.comboIcon5, questData.Icons[4]);

    // Objectives
    const auto setFieldFromDataObjective = [this, setFieldFromData](const QComboBox* combo, Resources::QuestClearParam param, u16& value) {
        switch (param) {
        case Resources::QuestClearParam::None: [[fallthrough]];
        case Resources::QuestClearParam::EarnWycademyPoints: [[fallthrough]];
        case Resources::QuestClearParam::DeliverPawPass: [[fallthrough]];
        case Resources::QuestClearParam::BreakParts: [[fallthrough]];
        case Resources::QuestClearParam::MountAndTopple:
            break;
        case Resources::QuestClearParam::Hunt: [[fallthrough]];
        case Resources::QuestClearParam::Capture: [[fallthrough]];
        case Resources::QuestClearParam::HuntAllLargeMonsters: [[fallthrough]];
        case Resources::QuestClearParam::SlayTotalOfTargets:
            setFieldFromData(combo, value);
            break;
        case Resources::QuestClearParam::DeliverItem:
            value = combo->currentIndex();
            break;
        }
    };

    setFieldFromData(ui.comboObjectiveType1, questData.ClearConditions[0].Param);
    setFieldFromData(ui.comboObjectiveType2, questData.ClearConditions[1].Param);
    setFieldFromData(ui.comboSubObjectiveType, questData.SubClearCondition.Param);

    setFieldFromDataObjective(ui.comboObjectiveId1, questData.ClearConditions[0].Param, questData.ClearConditions[0].Value);
    setFieldFromDataObjective(ui.comboObjectiveId2, questData.ClearConditions[1].Param, questData.ClearConditions[1].Value);
    setFieldFromDataObjective(ui.comboSubObjectiveId, questData.SubClearCondition.Param, questData.SubClearCondition.Value);

    questData.ClearConditions[0].Count = ui.spinBoxObjective1Amount->value();
    questData.ClearConditions[1].Count = ui.spinBoxObjective2Amount->value();
    questData.SubClearCondition.Count = ui.spinBoxSubObjectiveAmount->value();

    // Monsters
    setFieldFromData(ui.comboMonster1, questData.Monsters[0].Id);
    setFieldFromData(ui.comboMonster2, questData.Monsters[1].Id);
    setFieldFromData(ui.comboMonster3, questData.Monsters[2].Id);
    setFieldFromData(ui.comboMonster4, questData.Monsters[3].Id);
    setFieldFromData(ui.comboMonster5, questData.Monsters[4].Id);

    questData.Monsters[0].SubType = ui.spinBoxMonster1Special->value();
    questData.Monsters[1].SubType = ui.spinBoxMonster2Special->value();
    questData.Monsters[2].SubType = ui.spinBoxMonster3Special->value();
    questData.Monsters[3].SubType = ui.spinBoxMonster4Special->value();
    questData.Monsters[4].SubType = ui.spinBoxMonster5Special->value();

    questData.Monsters[0].AuraType = ui.spinBoxMonster1State->value();
    questData.Monsters[1].AuraType = ui.spinBoxMonster2State->value();
    questData.Monsters[2].AuraType = ui.spinBoxMonster3State->value();
    questData.Monsters[3].AuraType = ui.spinBoxMonster4State->value();
    questData.Monsters[4].AuraType = ui.spinBoxMonster5State->value();

    questData.Monsters[0].RestoreAmount = ui.spinBoxMonster1RestoreAmount->value();
    questData.Monsters[1].RestoreAmount = ui.spinBoxMonster2RestoreAmount->value();
    questData.Monsters[2].RestoreAmount = ui.spinBoxMonster3RestoreAmount->value();
    questData.Monsters[3].RestoreAmount = ui.spinBoxMonster4RestoreAmount->value();
    questData.Monsters[4].RestoreAmount = ui.spinBoxMonster5RestoreAmount->value();

    setFieldFromIndex(ui.comboMonster1Health, questData.Monsters[0].HealthTableIndex);
    setFieldFromIndex(ui.comboMonster2Health, questData.Monsters[1].HealthTableIndex);
    setFieldFromIndex(ui.comboMonster3Health, questData.Monsters[2].HealthTableIndex);
    setFieldFromIndex(ui.comboMonster4Health, questData.Monsters[3].HealthTableIndex);
    setFieldFromIndex(ui.comboMonster5Health, questData.Monsters[4].HealthTableIndex);

    setFieldFromIndex(ui.comboMonster1Attack, questData.Monsters[0].AttackTableIndex);
    setFieldFromIndex(ui.comboMonster2Attack, questData.Monsters[1].AttackTableIndex);
    setFieldFromIndex(ui.comboMonster3Attack, questData.Monsters[2].AttackTableIndex);
    setFieldFromIndex(ui.comboMonster4Attack, questData.Monsters[3].AttackTableIndex);
    setFieldFromIndex(ui.comboMonster5Attack, questData.Monsters[4].AttackTableIndex);

    setFieldFromIndex(ui.comboMonster1Other, questData.Monsters[0].HealthTableIndex);
    setFieldFromIndex(ui.comboMonster2Other, questData.Monsters[1].HealthTableIndex);
    setFieldFromIndex(ui.comboMonster3Other, questData.Monsters[2].HealthTableIndex);
    setFieldFromIndex(ui.comboMonster4Other, questData.Monsters[3].HealthTableIndex);
    setFieldFromIndex(ui.comboMonster5Other, questData.Monsters[4].HealthTableIndex);

    questData.Monsters[0].Difficulty = ui.spinBoxMonster1Difficulty->value();
    questData.Monsters[1].Difficulty = ui.spinBoxMonster2Difficulty->value();
    questData.Monsters[2].Difficulty = ui.spinBoxMonster3Difficulty->value();
    questData.Monsters[3].Difficulty = ui.spinBoxMonster4Difficulty->value();
    questData.Monsters[4].Difficulty = ui.spinBoxMonster5Difficulty->value();

    questData.Monsters[0].Size = ui.spinBoxMonster1Size->value();
    questData.Monsters[1].Size = ui.spinBoxMonster2Size->value();
    questData.Monsters[2].Size = ui.spinBoxMonster3Size->value();
    questData.Monsters[3].Size = ui.spinBoxMonster4Size->value();
    questData.Monsters[4].Size = ui.spinBoxMonster5Size->value();

    questData.Monsters[0].SizeTableIndex = ui.spinBoxMonster1SizeTable->value();
    questData.Monsters[1].SizeTableIndex = ui.spinBoxMonster2SizeTable->value();
    questData.Monsters[2].SizeTableIndex = ui.spinBoxMonster3SizeTable->value();
    questData.Monsters[3].SizeTableIndex = ui.spinBoxMonster4SizeTable->value();
    questData.Monsters[4].SizeTableIndex = ui.spinBoxMonster5SizeTable->value();

    questData.Monsters[0].StaminaTableIndex = ui.spinBoxMonster1StaminaTable->value();
    questData.Monsters[1].StaminaTableIndex = ui.spinBoxMonster2StaminaTable->value();
    questData.Monsters[2].StaminaTableIndex = ui.spinBoxMonster3StaminaTable->value();
    questData.Monsters[3].StaminaTableIndex = ui.spinBoxMonster4StaminaTable->value();
    questData.Monsters[4].StaminaTableIndex = ui.spinBoxMonster5StaminaTable->value();

    setFieldFromIndex(ui.comboZakoHealth, questData.SmallMonsterHpIndex);
    setFieldFromIndex(ui.comboZakoAttack, questData.SmallMonsterAtkIndex);
    setFieldFromIndex(ui.comboZakoOther, questData.SmallMonsterOtherIndex);

    // Items
    questData.Supplies[0].SuppLabel = ui.spinBoxSuppliesALabel->value();
    questData.Supplies[0].SuppType = ui.spinBoxSuppliesAType->value();
    questData.Supplies[0].SuppTarget = ui.spinBoxSuppliesATarget->value();
    questData.Supplies[0].SuppTargetCount = ui.spinBoxSuppliesATargetAmount->value();

    questData.Supplies[1].SuppLabel = ui.spinBoxSuppliesBLabel->value();
    questData.Supplies[1].SuppType = ui.spinBoxSuppliesBType->value();
    questData.Supplies[1].SuppTarget = ui.spinBoxSuppliesBTarget->value();
    questData.Supplies[1].SuppTargetCount = ui.spinBoxSuppliesBTargetAmount->value();

    setFieldFromData(ui.comboCarveLevel, questData.CarveLevel);
    setFieldFromData(ui.comboGatheringLevel, questData.GatheringLevel);
    setFieldFromData(ui.comboFishingLevel, questData.FishingLevel);

    questData.RemAddFrame[0] = ui.spinBoxRewardBoxes1->value();
    questData.RemAddFrame[1] = ui.spinBoxRewardBoxes2->value();
    questData.RemAddLotMax = ui.spinBoxMaxExtraBoxes->value();

    // Spawns
    questData.EnemySet2.SetType = ui.spinBoxMonster2SetType->value();
    questData.EnemySet2.TargetId = ui.spinBoxMonster2SetTargetId->value();
    questData.EnemySet2.TargetCount = ui.spinBoxMonster2SetTargetAmount->value();

    questData.EnemySet3.SetType = ui.spinBoxMonster3SetType->value();
    questData.EnemySet3.TargetId = ui.spinBoxMonster3SetTargetId->value();
    questData.EnemySet3.TargetCount = ui.spinBoxMonster3SetTargetAmount->value();

    questData.MonsterSpawns[0].SpawnType = ui.spinBoxMonster1SpawnType->value();
    questData.MonsterSpawns[1].SpawnType = ui.spinBoxMonster2SpawnType->value();
    questData.MonsterSpawns[2].SpawnType = ui.spinBoxMonster3SpawnType->value();
    questData.MonsterSpawns[3].SpawnType = ui.spinBoxMonster4SpawnType->value();
    questData.MonsterSpawns[4].SpawnType = ui.spinBoxMonster5SpawnType->value();

    questData.MonsterSpawns[0].SpawnTargetType = ui.spinBoxMonster1SpawnTargetType->value();
    questData.MonsterSpawns[1].SpawnTargetType = ui.spinBoxMonster2SpawnTargetType->value();
    questData.MonsterSpawns[2].SpawnTargetType = ui.spinBoxMonster3SpawnTargetType->value();
    questData.MonsterSpawns[3].SpawnTargetType = ui.spinBoxMonster4SpawnTargetType->value();
    questData.MonsterSpawns[4].SpawnTargetType = ui.spinBoxMonster5SpawnTargetType->value();

    questData.MonsterSpawns[0].SpawnTargetCount = ui.spinBoxMonster1SpawnTargetAmount->value();
    questData.MonsterSpawns[1].SpawnTargetCount = ui.spinBoxMonster2SpawnTargetAmount->value();
    questData.MonsterSpawns[2].SpawnTargetCount = ui.spinBoxMonster3SpawnTargetAmount->value();
    questData.MonsterSpawns[3].SpawnTargetCount = ui.spinBoxMonster4SpawnTargetAmount->value();
    questData.MonsterSpawns[4].SpawnTargetCount = ui.spinBoxMonster5SpawnTargetAmount->value();

    questData.StrayRand = ui.spinBoxStrayRandom->value();
    questData.StrayStartTime = ui.spinBoxStrayStart->value();
    questData.StrayStartRand = ui.spinBoxStrayStartRandom->value();
    questData.StrayLimit345[0] = ui.spinBoxStrayLimit3->value();
    questData.StrayLimit345[1] = ui.spinBoxStrayLimit4->value();
    questData.StrayLimit345[2] = ui.spinBoxStrayLimit5->value();
    questData.StrayRand345[0] = ui.spinBoxStrayRandom3->value();
    questData.StrayRand345[1] = ui.spinBoxStrayRandom4->value();
    questData.StrayRand345[2] = ui.spinBoxStrayRandom5->value();
    questData.ExtraTicketCount = ui.spinBoxExtraTicketCount->value();
}

void MHGUQuestEditor::saveQuestInfoFromUi()
{
    for (s32 language = 0; language < gmds.size(); ++language)
    {
        if (language >= gmds.size() || !ui.tabWidgetLanguage->isTabEnabled(language))
            continue;
        auto& gmd = gmds[language];
        const auto tab = ui.tabWidgetLanguage->widget(language);
        const auto languageName = Resources::Language::toString(language, true);

        const auto textName = tab->findChild<QLineEdit*>("textName" + languageName);
        const auto textClient = tab->findChild<QLineEdit*>("textClient" + languageName);
        const auto textDescription = tab->findChild<QPlainTextEdit*>("textDescription" + languageName);
        const auto textZako = tab->findChild<QPlainTextEdit*>("textZako" + languageName);
        const auto textObjective = tab->findChild<QPlainTextEdit*>("textObjective" + languageName);
        const auto textFailure = tab->findChild<QPlainTextEdit*>("textFailure" + languageName);
        const auto textSubquest = tab->findChild<QLineEdit*>("textSubquest" + languageName);

        gmd.Entries[0] = textName->text().toStdString();
        gmd.Entries[1] = textClient->text().toStdString();
        gmd.Entries[2] = textDescription->toPlainText().replace("\n", "\r\n").toStdString();
        gmd.Entries[3] = textZako->toPlainText().replace("\n", "\r\n").toStdString();
        gmd.Entries[4] = textObjective->toPlainText().replace("\n", "\r\n").toStdString();
        gmd.Entries[5] = textFailure->toPlainText().replace("\n", "\r\n").toStdString();
        gmd.Entries[6] = textSubquest->text().toStdString();
    }
}

void MHGUQuestEditor::saveRemFromUi(Resources::Rem& rem, const QString& remName, s32 tabIndex)
{
    const auto tab = ui.tabWidgetRewards->widget(tabIndex);

    for (s32 i = 0; i < 8; ++i)
    {
        rem.Flags[i].Flag = (u8)tab->findChild<QSpinBox*>(QStringLiteral("spinBoxRem%1Flag%2").arg(remName).arg(i))->value();
        rem.Flags[i].Value = (u8)tab->findChild<QSpinBox*>(QStringLiteral("spinBoxRem%1Value%2").arg(remName).arg(i))->value();
    }

    const auto scrollArea = tab->findChild<QScrollArea*>(QStringLiteral("scrollAreaRem%1").arg(remName));
    const auto layout = (QVBoxLayout*)scrollArea->widget()->layout();

    std::memset(rem.Rewards, 0, sizeof(rem.Rewards));

    s32 i;
    for (i = 0; i < layout->count(); ++i)
    {
        const auto itemLayout = (QHBoxLayout*)layout->itemAt(i)->layout();
        const auto itemCombo = (QComboBox*)itemLayout->itemAt(1)->widget();
        const auto amountBox = (QSpinBox*)itemLayout->itemAt(2)->widget();
        const auto chanceBox = (QSpinBox*)itemLayout->itemAt(3)->widget();
        rem.Rewards[i].ItemId = itemCombo->currentIndex();
        rem.Rewards[i].Amount = amountBox->value();
        rem.Rewards[i].Weight = chanceBox->value();
    }

    if (i != 0 && i < std::size(rem.Rewards))
        rem.Rewards[i] = { .Weight = 255 }; // End marker
}

void MHGUQuestEditor::loadRemIntoUi(const Resources::Rem& rem, const QString& remName, s32 tabIndex)
{
    const auto tab = ui.tabWidgetRewards->widget(tabIndex);

    for (s32 i = 0; i < 8; ++i)
    {
        tab->findChild<QSpinBox*>(QStringLiteral("spinBoxRem%1Flag%2").arg(remName).arg(i))->setValue(rem.Flags[i].Flag);
        tab->findChild<QSpinBox*>(QStringLiteral("spinBoxRem%1Value%2").arg(remName).arg(i))->setValue(rem.Flags[i].Value);
    }

    const auto scrollArea = tab->findChild<QScrollArea*>(QStringLiteral("scrollAreaRem%1").arg(remName));
    const auto container = new QWidget;
    scrollArea->setWidget(container);
    const auto layout = new QVBoxLayout(container);

    if (*(const u32*)&rem.Rewards[0] == 0) // Empty rewards
        return;

    scrollArea->setUpdatesEnabled(false);

    for (const auto [i, item] : std::views::enumerate(rem.Rewards)) {
        if (item.ItemId == 0 && item.Amount == 0 && item.Weight == 255) // End marker
            break;

        const auto deleteButton = new QToolButton;
        const auto itemCombo = new QComboBox;
        const auto amountBox = new QSpinBox;
        const auto chanceBox = new QSpinBox;
        const auto itemLayout = new QHBoxLayout;
        
        itemCombo->setModel(itemNamesModel);
        chanceBox->setSuffix("%");
        
        deleteButton->setText("⨯");
        connect(deleteButton, &QToolButton::clicked, [itemLayout] {
            itemLayout->itemAt(0)->widget()->deleteLater(); // Delete button
            itemLayout->itemAt(1)->widget()->deleteLater(); // Item combo
            itemLayout->itemAt(2)->widget()->deleteLater(); // Amount box
            itemLayout->itemAt(3)->widget()->deleteLater(); // Chance box
            itemLayout->deleteLater();
        });

        itemCombo->setCurrentIndex(item.ItemId);
        amountBox->setValue(item.Amount);
        chanceBox->setValue(item.Weight);

        itemLayout->addWidget(deleteButton, 0);
        itemLayout->addWidget(itemCombo, 2);
        itemLayout->addWidget(amountBox, 1);
        itemLayout->addWidget(chanceBox, 1);
        layout->addLayout(itemLayout);
    }

    scrollArea->setUpdatesEnabled(true);
}

void MHGUQuestEditor::openSettings()
{
    const auto settings = new SettingsDialog(this, questListPath, autoUpdateQuestList);
    settings->exec();

    if (settings->result() == QDialog::Accepted)
    {
        qDebug("Settings updated");
        questListPath = settings->getQuestListPath();
        autoUpdateQuestList = settings->getAutoUpdateQuestList();
        saveSettings();
    }
}
