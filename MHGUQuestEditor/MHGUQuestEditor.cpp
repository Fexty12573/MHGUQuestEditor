#include "MHGUQuestEditor.h"

#include <ranges>

#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDragEnterEvent>
#include <QMimeData>

#include "Resources/Arc.h"
#include "Resources/QuestData.h"
#include "Resources/StatTable.h"

template<typename T> concept Enum = std::is_enum_v<T>;
template<typename T> concept Integral = std::is_integral_v<T>;
template<typename T> concept AnyIntegral = Integral<T> || Enum<T>;

MHGUQuestEditor::MHGUQuestEditor(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    setAcceptDrops(true);

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

    connect(ui.actionOpen, &QAction::triggered, this, &MHGUQuestEditor::onOpenFile);
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
    loadQuestFile(url.toLocalFile());
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

    //ui.labelIcon1->setPixmap(monsterIcons[1]);
    //ui.labelIcon2->setPixmap(monsterIcons[21]);
    //ui.labelIcon3->setPixmap(monsterIcons[111]);
    //ui.labelIcon4->setPixmap(monsterIcons[61]);
    //ui.labelIcon5->setPixmap(monsterIcons[106]);

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
            for (auto i = 0; i < ui.comboMonster1->count(); ++i) {
                idCombo->addItem(ui.comboMonster1->itemText(i), ui.comboMonster1->itemData(i));
            }
            break;
        case Resources::QuestClearParam::DeliverItem:
            idCombo->addItems(itemNames);
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
}

void MHGUQuestEditor::onOpenFile()
{
    // Open file dialog
    const auto path = QFileDialog::getOpenFileName(
        this, 
        "Open Quest File", 
        {}, 
        "Quest Files (*.mib *.ext);;Archive Files (*.arc)"
    );

    if (path.isEmpty())
        return;

    loadQuestFile(path);
}

void MHGUQuestEditor::onSaveFile()
{
}

void MHGUQuestEditor::loadQuestFile(const QString& path)
{
    QFile file(path.trimmed());
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical("Failed to open file %s", qUtf8Printable(path));
        return;
    }

    if (path.endsWith(".mib") || path.endsWith(".ext"))
    {
        questData = Resources::QuestData::deserialize(file.readAll());
    }
    else if (path.endsWith(".arc"))
    {
        qWarning("Loading from archives is not supported yet");
        //arc = std::make_unique<Resources::Arc>(path.toStdWString());
        //if (arc->getEntries().empty())
        //{
        //    qCritical("No entries found in %s", qUtf8Printable(path));
        //    return;
        //}
    }
    else
    {
        qCritical("Invalid file extension %s", qUtf8Printable(path));
        return;
    }

    loadQuestDataIntoUi();
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
