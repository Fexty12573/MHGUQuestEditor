#include "EquipSetEditor.h"
#include "MHGUQuestEditor.h"
#include "Resources/Gmd.h"

#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <ranges>

enum Roles {
    HunterArtIdRole = Qt::UserRole + 1,

    ArmorIdRole = Qt::UserRole + 1,

    WeaponFamilyRole = Qt::UserRole + 1,
    WeaponLevelRole = Qt::UserRole + 2,
};


EquipSetEditor::EquipSetEditor(QWidget *parent)
    : QWidget(parent)
{

    if (!HunterArtsModel)
    {
        QFile hunterArtsFile(":/res/hunter_arts.json");
        if (!hunterArtsFile.open(QIODevice::ReadOnly))
        {
            qFatal("Failed to open hunter_arts.json");
        }

        HunterArtsModel = new QStandardItemModel();

        const auto hunterArts = QJsonDocument::fromJson(hunterArtsFile.readAll()).object();
        for (const auto& key : hunterArts.keys())
        {
            const auto id = hunterArts[key].toInt();
            const auto item = new QStandardItem(key);
            item->setData(id, Roles::HunterArtIdRole);
            HunterArtsModel->appendRow(item);
        }

        QFile armorSeriesDataFile(":/res/armor_series_data.json");
        QFile armorNamesFile(":/res/armor_series_data.gmd");

        if (!armorSeriesDataFile.open(QIODevice::ReadOnly))
        {
            qFatal("Failed to open armor_series_data.json");
        }

        if (!armorNamesFile.open(QIODevice::ReadOnly))
        {
            qFatal("Failed to open armor_series_data.gmd");
        }

        const auto armorNames = Resources::Gmd::deserialize(armorNamesFile.readAll());
        const auto armorSeriesData = QJsonDocument::fromJson(armorSeriesDataFile.readAll()).array();

        // armor_series_data.gmd format is:
        // 0: Head Name
        // 1: Chest Name
        // 2: Arm Name
        // 3: Waist Name
        // 4: Leg Name
        // 5: Head Description
        // 6: Chest Description
        // 7: Arm Description
        // 8: Waist Description
        // 9: Leg Description
        if (armorNames.Entries.size() / 10 != armorSeriesData.size())
        {
            qFatal("Armor names and armor series data do not match");
        }

        HeadArmorModel = new QStandardItemModel();
        ChestArmorModel = new QStandardItemModel();
        ArmArmorModel = new QStandardItemModel();
        WaistArmorModel = new QStandardItemModel();
        LegArmorModel = new QStandardItemModel();

        ArmorModels[0] = HeadArmorModel;
        ArmorModels[1] = ChestArmorModel;
        ArmorModels[2] = ArmArmorModel;
        ArmorModels[3] = WaistArmorModel;
        ArmorModels[4] = LegArmorModel;

        QFontMetrics fm(QFont("Segoe UI", 11));

        for (const auto armor : armorSeriesData)
        {
            const auto obj = armor.toObject();
            const auto id = obj["Id"].toInt();
            const auto maxLevel = obj["MaxLevel"].toInt();

            ArmorSeriesData.emplace_back(id, maxLevel);

            const auto pieces = obj["Pieces"].toArray();
            if (pieces[0].toInt()) {
                const auto name = QString::fromStdString(armorNames.Entries[id * 10ull + 0]);
                const auto item = new QStandardItem(name);
                item->setData(id, Roles::ArmorIdRole);
                HeadArmorModel->appendRow(item);
                ArmorComboMinWidth = std::max(ArmorComboMinWidth, fm.horizontalAdvance(name) + 15);
            }

            if (pieces[1].toInt()) {
                const auto name = QString::fromStdString(armorNames.Entries[id * 10ull + 1]);
                const auto item = new QStandardItem(name);
                item->setData(id, Roles::ArmorIdRole);
                ChestArmorModel->appendRow(item);
                ArmorComboMinWidth = std::max(ArmorComboMinWidth, fm.horizontalAdvance(name) + 15);
            }

            if (pieces[2].toInt()) {
                const auto name = QString::fromStdString(armorNames.Entries[id * 10ull + 2]);
                const auto item = new QStandardItem(name);
                item->setData(id, Roles::ArmorIdRole);
                ArmArmorModel->appendRow(item);
                ArmorComboMinWidth = std::max(ArmorComboMinWidth, fm.horizontalAdvance(name) + 15);
            }

            if (pieces[3].toInt()) {
                const auto name = QString::fromStdString(armorNames.Entries[id * 10ull + 3]);
                const auto item = new QStandardItem(name);
                item->setData(id, Roles::ArmorIdRole);
                WaistArmorModel->appendRow(item);
                ArmorComboMinWidth = std::max(ArmorComboMinWidth, fm.horizontalAdvance(name) + 15);
            }

            if (pieces[4].toInt()) {
                const auto name = QString::fromStdString(armorNames.Entries[id * 10ull + 4]);
                const auto item = new QStandardItem(name);
                item->setData(id, Roles::ArmorIdRole);
                LegArmorModel->appendRow(item);
                ArmorComboMinWidth = std::max(ArmorComboMinWidth, fm.horizontalAdvance(name) + 15);
            }
        }

        SkillNamesModel = new QStringListModel();

        QFile skillNamesFile(":/res/skill_names.json");
        if (!skillNamesFile.open(QIODevice::ReadOnly))
        {
            qFatal("Failed to open skill_names.json");
        }

        const auto skillNames = QJsonDocument::fromJson(skillNamesFile.readAll());
        SkillNamesModel->setStringList(skillNames.toVariant().toStringList());

        QFile weaponFile(":/res/weapons.json");
        if (!weaponFile.open(QIODevice::ReadOnly))
        {
            qFatal("Failed to open weapons.json");
        }

        using Resources::AcEquip;
        QJsonArray weapons = QJsonDocument::fromJson(weaponFile.readAll()).array();

        for (const auto& [id, name] : AcEquip::WeaponTypes)
        {
            const auto weaponModel = new QStandardItemModel();
            WeaponModels[id] = weaponModel;
            const auto weaponData = std::ranges::find_if(weapons, [&name](const auto weapon) {
                return weapon.toObject()["Type"].toString() == name;
            })->toObject()["Weapons"].toArray();

            const auto noneItem = new QStandardItem("None");
            noneItem->setData(0, Roles::WeaponFamilyRole);
            noneItem->setData(0, Roles::WeaponLevelRole);
            weaponModel->appendRow(noneItem);

            for (const auto weaponRef : weaponData)
            {
                const auto weapon = weaponRef.toObject();
                const auto item = new QStandardItem(weapon["Name"].toString());
                item->setData(weapon["Family"].toInt(), Roles::WeaponFamilyRole);
                item->setData(weapon["Level"].toInt(), Roles::WeaponLevelRole);
                weaponModel->appendRow(item);
            }
        }
    }

    ui.setupUi(this);

    ui.comboWeaponDeco1->setModel(new DecorationSortFilterProxyModel(MHGUQuestEditor::ItemNamesModel));
    ui.comboWeaponDeco2->setModel(new DecorationSortFilterProxyModel(MHGUQuestEditor::ItemNamesModel));
    ui.comboWeaponDeco3->setModel(new DecorationSortFilterProxyModel(MHGUQuestEditor::ItemNamesModel));

    ui.comboCharmDeco1->setModel(new DecorationSortFilterProxyModel(MHGUQuestEditor::ItemNamesModel));
    ui.comboCharmDeco2->setModel(new DecorationSortFilterProxyModel(MHGUQuestEditor::ItemNamesModel));
    ui.comboCharmDeco3->setModel(new DecorationSortFilterProxyModel(MHGUQuestEditor::ItemNamesModel));

    ui.comboCharmDeco1->view()->setMinimumWidth(190);
    ui.comboCharmDeco2->view()->setMinimumWidth(190);
    ui.comboCharmDeco3->view()->setMinimumWidth(190);

    for (const auto& [id, name] : Resources::AcEquip::WeaponTypes) {
        ui.comboWeaponType->addItem(name, id);
    }

    for (const auto& [id, name] : Resources::AcEquip::Styles) {
        ui.comboStyle->addItem(name, id);
    }

    ui.comboWeaponType->setCurrentIndex(ui.comboWeaponType->findData(Resources::AcEquip::GreatSword));
    ui.comboWeapon->setModel(WeaponModels[Resources::AcEquip::GreatSword]);

    ui.comboHunterArt1->setModel(HunterArtsModel);
    ui.comboHunterArt2->setModel(HunterArtsModel);
    ui.comboHunterArt3->setModel(HunterArtsModel);

    ui.comboCharmSkill1->setModel(SkillNamesModel);
    ui.comboCharmSkill2->setModel(SkillNamesModel);

    static constexpr std::array armorPieces = {
        "Head",
        "Chest",
        "Arms",
        "Waist",
        "Legs"
    };

    for (int i = 0; i < armorPieces.size(); ++i)
    {
        ui.tableWidgetArmor->insertRow(i);

        ui.tableWidgetArmor->setCellWidget(i, 0, new QLabel(armorPieces[i]));

        const auto& armorSeries = ArmorSeriesData[0];

        const auto armorCombo = new QComboBox(ui.tableWidgetArmor);
        armorCombo->setModel(ArmorModels[i]);
        armorCombo->setCurrentIndex(armorCombo->findData(0));
        armorCombo->view()->setMinimumWidth(ArmorComboMinWidth);

        ui.tableWidgetArmor->setCellWidget(i, 1, armorCombo);

        const auto levelBox = new QSpinBox(ui.tableWidgetArmor);
        levelBox->setRange(1, armorSeries.MaxLevel);
        levelBox->setValue(1);

        ui.tableWidgetArmor->setCellWidget(i, 2, levelBox);

        const auto deco1Combo = new QComboBox(ui.tableWidgetArmor);
        const auto deco2Combo = new QComboBox(ui.tableWidgetArmor);
        const auto deco3Combo = new QComboBox(ui.tableWidgetArmor);

        deco1Combo->setModel(new DecorationSortFilterProxyModel(MHGUQuestEditor::ItemNamesModel));
        deco2Combo->setModel(new DecorationSortFilterProxyModel(MHGUQuestEditor::ItemNamesModel));
        deco3Combo->setModel(new DecorationSortFilterProxyModel(MHGUQuestEditor::ItemNamesModel));

        deco1Combo->setCurrentIndex(0);
        deco2Combo->setCurrentIndex(0);
        deco3Combo->setCurrentIndex(0);

        deco1Combo->view()->setMinimumWidth(190);
        deco2Combo->view()->setMinimumWidth(190);
        deco3Combo->view()->setMinimumWidth(190);

        ui.tableWidgetArmor->setCellWidget(i, 3, deco1Combo);
        ui.tableWidgetArmor->setCellWidget(i, 4, deco2Combo);
        ui.tableWidgetArmor->setCellWidget(i, 5, deco3Combo);
    }

    for (int i = 0; i < std::size(Resources::EquipSet{}.Items); ++i)
    {
        ui.tableWidgetItems->insertRow(i);

        const auto itemCombo = new QComboBox(ui.tableWidgetItems);
        itemCombo->setModel(MHGUQuestEditor::ItemNamesModel);
        itemCombo->setCurrentIndex(0);
        itemCombo->setMinimumWidth(190);
        itemCombo->view()->setMinimumWidth(190);
        ui.tableWidgetItems->setCellWidget(i, 0, itemCombo);

        const auto countBox = new QSpinBox(ui.tableWidgetItems);
        countBox->setRange(0, 99);
        countBox->setValue(0);
        ui.tableWidgetItems->setCellWidget(i, 1, countBox);
    }

    connect(ui.comboWeaponType, &QComboBox::currentIndexChanged, this, [this](int) {
        const auto type = ui.comboWeaponType->currentData().toInt();
        ui.comboWeapon->setModel(WeaponModels[type]);
        ui.comboWeapon->setCurrentIndex(0);
        if (equipSet) equipSet->WeaponType = type;
    });
    connect(ui.comboWeapon, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (equipSet)
        {
            const auto weaponModel = qobject_cast<const QStandardItemModel*>(ui.comboWeapon->model());
            const auto idx = weaponModel->index(index, 0);
            equipSet->WeaponId = idx.data(Roles::WeaponFamilyRole).toInt();
            equipSet->WeaponLevel = idx.data(Roles::WeaponLevelRole).toInt();
        }
    });
    connect(ui.comboStyle, &QComboBox::currentIndexChanged, this, [this](int) {
        if (equipSet) equipSet->Style = ui.comboStyle->currentData().toInt();
    });
    connect(ui.spinBoxCustomAttr, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (equipSet) equipSet->CustomAttr = value;
    });
    connect(ui.comboHunterArt1, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (equipSet) equipSet->HunterArts[0] = ui.comboHunterArt1->model()->index(index, 0).data(Roles::HunterArtIdRole).toInt();
    });
    connect(ui.comboHunterArt2, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (equipSet) equipSet->HunterArts[1] = ui.comboHunterArt2->model()->index(index, 0).data(Roles::HunterArtIdRole).toInt();
    });
    connect(ui.comboHunterArt3, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (equipSet) equipSet->HunterArts[2] = ui.comboHunterArt3->model()->index(index, 0).data(Roles::HunterArtIdRole).toInt();
    });
    connect(ui.comboWeaponDeco1, &QComboBox::currentIndexChanged, this, [this](int index) {
        const auto model = qobject_cast<const DecorationSortFilterProxyModel*>(ui.comboWeaponDeco1->model());
        if (equipSet) equipSet->WeaponDecos[0] = model->mapToSource(model->index(index, 0)).row();
    });
    connect(ui.comboWeaponDeco2, &QComboBox::currentIndexChanged, this, [this](int index) {
        const auto model = qobject_cast<const DecorationSortFilterProxyModel*>(ui.comboWeaponDeco2->model());
        if (equipSet) equipSet->WeaponDecos[1] = model->mapToSource(model->index(index, 0)).row();
    });
    connect(ui.comboWeaponDeco3, &QComboBox::currentIndexChanged, this, [this](int index) {
        const auto model = qobject_cast<const DecorationSortFilterProxyModel*>(ui.comboWeaponDeco3->model());
        if (equipSet) equipSet->WeaponDecos[2] = model->mapToSource(model->index(index, 0)).row();
    });
    connect(ui.comboCharmSkill1, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (equipSet) equipSet->CharmSkill1 = index;
    });
    connect(ui.comboCharmSkill2, &QComboBox::currentIndexChanged, this, [this](int index) {
        if (equipSet) equipSet->CharmSkill2 = index;
    });
    connect(ui.spinBoxCharmSkil1Level, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (equipSet) equipSet->CharmSkill1Level = value;
    });
    connect(ui.spinBoxCharmSkil2Level, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        if (equipSet) equipSet->CharmSkill2Level = value;
    });
    connect(ui.comboCharmDeco1, &QComboBox::currentIndexChanged, this, [this](int index) {
        const auto model = qobject_cast<const DecorationSortFilterProxyModel*>(ui.comboCharmDeco1->model());
        if (equipSet) equipSet->Equipment[5].Decos[0] = model->mapToSource(model->index(index, 0)).row();
    });
    connect(ui.comboCharmDeco2, &QComboBox::currentIndexChanged, this, [this](int index) {
        const auto model = qobject_cast<const DecorationSortFilterProxyModel*>(ui.comboCharmDeco2->model());
        if (equipSet) equipSet->Equipment[5].Decos[1] = model->mapToSource(model->index(index, 0)).row();
    });
    connect(ui.comboCharmDeco3, &QComboBox::currentIndexChanged, this, [this](int index) {
        const auto model = qobject_cast<const DecorationSortFilterProxyModel*>(ui.comboCharmDeco3->model());
        if (equipSet) equipSet->Equipment[5].Decos[2] = model->mapToSource(model->index(index, 0)).row();
    });

    for (int i = 0; i < std::size(Resources::EquipSet{}.Equipment) - 1; ++i)
    {
        const auto armorCombo = qobject_cast<QComboBox*>(ui.tableWidgetArmor->cellWidget(i, 1));
        connect(armorCombo, &QComboBox::currentIndexChanged, this, [this, i, armorCombo](int index) {
            if (equipSet)
            {
                const auto model = qobject_cast<const QStandardItemModel*>(armorCombo->model());
                const auto idx = model->index(index, 0);
                equipSet->Equipment[i].Id = idx.data(Roles::ArmorIdRole).toInt();

                const auto maxLevel = ArmorSeriesData[equipSet->Equipment[i].Id].MaxLevel;
                qobject_cast<QSpinBox*>(ui.tableWidgetArmor->cellWidget(i, 2))->setRange(1, maxLevel);
            }
        });

        const auto levelBox = qobject_cast<QSpinBox*>(ui.tableWidgetArmor->cellWidget(i, 2));
        connect(levelBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, i](int value) {
            if (equipSet) equipSet->Equipment[i].Level = value;
        });

        const auto deco1Combo = qobject_cast<QComboBox*>(ui.tableWidgetArmor->cellWidget(i, 3));
        connect(deco1Combo, &QComboBox::currentIndexChanged, this, [this, i, deco1Combo](int index) {
            const auto model = qobject_cast<const DecorationSortFilterProxyModel*>(deco1Combo->model());
            if (equipSet) equipSet->Equipment[i].Decos[0] = model->mapToSource(model->index(index, 0)).row();
        });

        const auto deco2Combo = qobject_cast<QComboBox*>(ui.tableWidgetArmor->cellWidget(i, 4));
        connect(deco2Combo, &QComboBox::currentIndexChanged, this, [this, i, deco2Combo](int index) {
            const auto model = qobject_cast<const DecorationSortFilterProxyModel*>(deco2Combo->model());
            if (equipSet) equipSet->Equipment[i].Decos[1] = model->mapToSource(model->index(index, 0)).row();
        });

        const auto deco3Combo = qobject_cast<QComboBox*>(ui.tableWidgetArmor->cellWidget(i, 5));
        connect(deco3Combo, &QComboBox::currentIndexChanged, this, [this, i, deco3Combo](int index) {
            const auto model = qobject_cast<const DecorationSortFilterProxyModel*>(deco3Combo->model());
            if (equipSet) equipSet->Equipment[i].Decos[2] = model->mapToSource(model->index(index, 0)).row();
        });
    }

    for (int i = 0; i < std::size(Resources::EquipSet{}.Items); ++i)
    {
        const auto itemCombo = qobject_cast<QComboBox*>(ui.tableWidgetItems->cellWidget(i, 0));
        connect(itemCombo, &QComboBox::currentIndexChanged, this, [this, i, itemCombo](int index) {
            if (equipSet) equipSet->Items[i].Id = index;
        });
        const auto countBox = qobject_cast<QSpinBox*>(ui.tableWidgetItems->cellWidget(i, 1));
        connect(countBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, i](int value) {
            if (equipSet) equipSet->Items[i].Count = value;
        });
    }
}

void EquipSetEditor::loadEquipSet()
{
    if (!equipSet)
        return;

    Resources::EquipSet before = *equipSet;

    const auto decoIdToIndex = [](const QAbstractItemModel* model, int id) {
        const auto decoModel = qobject_cast<const DecorationSortFilterProxyModel*>(model);
        return decoModel->mapFromSource(decoModel->sourceModel()->index(id, 0)).row();
    };

    const auto findItemIndex = [](const QComboBox* combo, int id) {
        const auto model = qobject_cast<const QStandardItemModel*>(combo->model());
        for (auto i = 0; i < model->rowCount(); ++i)
        {
            if (model->index(i, 0).data(Roles::ArmorIdRole).toInt() == id)
            {
                return i;
            }
        }

        return -1;
    };

    ui.comboWeaponType->blockSignals(true);
    ui.comboWeapon->blockSignals(true);

    ui.comboWeaponType->setCurrentIndex(ui.comboWeaponType->findData(equipSet->WeaponType));
    ui.comboWeapon->setModel(WeaponModels[equipSet->WeaponType]);
    const auto weaponModel = WeaponModels[equipSet->WeaponType];
    for (int i = 0; i < weaponModel->rowCount(); ++i)
    {
        const auto idx = weaponModel->index(i, 0);
        if (idx.data(Roles::WeaponFamilyRole).toInt() == equipSet->WeaponId && 
            idx.data(Roles::WeaponLevelRole).toInt() == equipSet->WeaponLevel)
        {
            ui.comboWeapon->setCurrentIndex(i);
            break;
        }
    }

    ui.comboStyle->setCurrentIndex(ui.comboStyle->findData(equipSet->Style));
    ui.spinBoxCustomAttr->setValue(equipSet->CustomAttr);
    ui.comboHunterArt1->setCurrentIndex(findItemIndex(ui.comboHunterArt1, equipSet->HunterArts[0]));
    ui.comboHunterArt2->setCurrentIndex(findItemIndex(ui.comboHunterArt2, equipSet->HunterArts[1]));
    ui.comboHunterArt3->setCurrentIndex(findItemIndex(ui.comboHunterArt3, equipSet->HunterArts[2]));
    ui.comboWeaponDeco1->setCurrentIndex(decoIdToIndex(ui.comboWeaponDeco1->model(), equipSet->WeaponDecos[0]));
    ui.comboWeaponDeco2->setCurrentIndex(decoIdToIndex(ui.comboWeaponDeco2->model(), equipSet->WeaponDecos[1]));
    ui.comboWeaponDeco3->setCurrentIndex(decoIdToIndex(ui.comboWeaponDeco3->model(), equipSet->WeaponDecos[2]));
    ui.comboCharmSkill1->setCurrentIndex(equipSet->CharmSkill1);
    ui.comboCharmSkill2->setCurrentIndex(equipSet->CharmSkill2);
    ui.spinBoxCharmSkil1Level->setValue(equipSet->CharmSkill1Level);
    ui.spinBoxCharmSkil2Level->setValue(equipSet->CharmSkill2Level);
    ui.comboCharmDeco1->setCurrentIndex(decoIdToIndex(ui.comboCharmDeco1->model(), equipSet->Equipment[5].Decos[0]));
    ui.comboCharmDeco2->setCurrentIndex(decoIdToIndex(ui.comboCharmDeco2->model(), equipSet->Equipment[5].Decos[1]));
    ui.comboCharmDeco3->setCurrentIndex(decoIdToIndex(ui.comboCharmDeco3->model(), equipSet->Equipment[5].Decos[2]));

    for (int i = 0; i < std::size(equipSet->Equipment) - 1; ++i)
    {
        const auto& equipment = equipSet->Equipment[i];

        const auto armorCombo = qobject_cast<QComboBox*>(ui.tableWidgetArmor->cellWidget(i, 1));
        armorCombo->setCurrentIndex(findItemIndex(armorCombo, equipment.Id));

        const auto levelBox = qobject_cast<QSpinBox*>(ui.tableWidgetArmor->cellWidget(i, 2));
        levelBox->setValue(equipment.Level);

        const auto deco1Combo = qobject_cast<QComboBox*>(ui.tableWidgetArmor->cellWidget(i, 3));
        deco1Combo->setCurrentIndex(decoIdToIndex(deco1Combo->model(), equipment.Decos[0]));

        const auto deco2Combo = qobject_cast<QComboBox*>(ui.tableWidgetArmor->cellWidget(i, 4));
        deco2Combo->setCurrentIndex(decoIdToIndex(deco2Combo->model(), equipment.Decos[1]));

        const auto deco3Combo = qobject_cast<QComboBox*>(ui.tableWidgetArmor->cellWidget(i, 5));
        deco3Combo->setCurrentIndex(decoIdToIndex(deco3Combo->model(), equipment.Decos[2]));
    }

    for (int i = 0; i < std::size(equipSet->Items); ++i)
    {
        const auto& item = equipSet->Items[i];

        const auto itemCombo = qobject_cast<QComboBox*>(ui.tableWidgetItems->cellWidget(i, 0));
        itemCombo->setCurrentIndex(item.Id);

        const auto countBox = qobject_cast<QSpinBox*>(ui.tableWidgetItems->cellWidget(i, 1));
        countBox->setValue(item.Count);
    }

    ui.comboWeaponType->blockSignals(false);
    ui.comboWeapon->blockSignals(false);

    Q_ASSERT(std::memcmp(&before, equipSet, sizeof before) == 0);
}
