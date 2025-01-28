#include "EmSetListEditor.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QTreeWidget>
#include <utility>
#include <ranges>

std::unordered_map<int, QString> EmSetListEditor::monsterNames;

enum ItemType
{
    Pack = QTreeWidgetItem::UserType + 1,
    Ems = QTreeWidgetItem::UserType + 2
};

EmSetListEditor::EmSetListEditor(Resources::EmSetList esl, QWidget *parent)
    : QWidget(parent), esl(std::move(esl))
{
    ui.setupUi(this);

    ui.spinBoxUnk4->setMinimum(std::numeric_limits<int32_t>::min());
    ui.spinBoxUnk4->setMaximum(std::numeric_limits<int32_t>::max());
    ui.spinBoxUnk5->setMinimum(std::numeric_limits<int32_t>::min());
    ui.spinBoxUnk5->setMaximum(std::numeric_limits<int32_t>::max());

    ui.buttonPasteAll->setEnabled(false);

    if (monsterNames.empty())
    {
        auto emsNames = QFile(":/res/ems_names.json");
        if (!emsNames.open(QIODevice::ReadOnly))
        {
            qFatal("Failed to open ems_names.json");
        }

        QJsonObject names = QJsonDocument::fromJson(emsNames.readAll()).object();
        for (const auto& name : names.keys())
        {
            monsterNames[names[name].toInt()] = name;
        }
    }

    for (const auto& [id, name] : monsterNames)
    {
        ui.comboMonster->addItem(name, id);
    }

#define CONNECT_SPINBOX(name, type, field, valtype, ...) \
    connect(ui.name, &type::valueChanged, this, [this](valtype _value) { \
        if (currentEms) { \
            currentEms->field = __VA_ARGS__ _value; \
        } \
    })

    connect(ui.comboMonster, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (currentEms) {
            currentEms->MonsterId = ui.comboMonster->currentData().toInt();
            ui.treeWidgetPackSelector->currentItem()->setText(0, formatEmsName(*currentEms));
        }
    });
    connect(ui.spinBoxArea, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        if (currentEms) {
            currentEms->Area = ui.spinBoxArea->value();
            ui.treeWidgetPackSelector->currentItem()->setText(0, formatEmsName(*currentEms));
        }
    });
    CONNECT_SPINBOX(spinBoxSpawnCond, QSpinBox, SpawnCondition, int);
    CONNECT_SPINBOX(doubleSpinBoxX, QDoubleSpinBox, Pos[0], double, (float));
    CONNECT_SPINBOX(doubleSpinBoxY, QDoubleSpinBox, Pos[1], double, (float));
    CONNECT_SPINBOX(doubleSpinBoxZ, QDoubleSpinBox, Pos[2], double, (float));
    CONNECT_SPINBOX(doubleSpinBoxAngle, QDoubleSpinBox, Angle, double, (float));
    CONNECT_SPINBOX(spinBoxUnk0, QSpinBox, Unk1[0], int);
    CONNECT_SPINBOX(spinBoxUnk1, QSpinBox, Unk1[1], int);
    CONNECT_SPINBOX(spinBoxUnk2, QSpinBox, Unk2[0], int);
    CONNECT_SPINBOX(spinBoxUnk3, QSpinBox, Unk2[1], int);
    CONNECT_SPINBOX(spinBoxUnk4, QSpinBox, Unk3[0], int);
    CONNECT_SPINBOX(spinBoxUnk5, QSpinBox, Unk3[1], int);

#undef CONNECT_SPINBOX

    connect(ui.buttonCopyAll, &QPushButton::clicked, this, [this] {
        copiedEms = *currentEms;
        ui.buttonPasteAll->setEnabled(true);
    });
    connect(ui.buttonPasteAll, &QPushButton::clicked, this, [this] {
        *currentEms = copiedEms;
        loadEmsIntoUi(*currentEms);
    });

    connect(ui.treeWidgetPackSelector, &QTreeWidget::customContextMenuRequested, this, [this](QPoint pos) {
        QMenu menu;

        // If right-click was on empty space then add the 'Add Pack' option
        // If right-click was on a 'pack' item then add the 'Add Ems' option
        // as well as the 'Remove Pack' option
        // If right-click was on an 'ems' item then add the 'Remove Ems' option
        auto item = ui.treeWidgetPackSelector->itemAt(pos);
        if (!item)
        {
            menu.addAction("Add Pack", this, [this] {
                this->esl.Packs.emplace_back();
                const auto packItem = new QTreeWidgetItem(ui.treeWidgetPackSelector, ItemType::Pack);
                packItem->setText(0, QStringLiteral("Pack %1").arg(this->esl.Packs.size()));
                packItem->setData(0, Qt::UserRole, this->esl.Packs.size() - 1);
            });
        }
        else if (item->type() == ItemType::Pack)
        {
            const auto packIndex = item->data(0, Qt::UserRole).toInt();
            menu.addAction("Add Monster", this, [this, item, packIndex] {
                auto& pack = this->esl.Packs[packIndex];
                pack.Ems.emplace_back();
                const auto emsItem = new QTreeWidgetItem(item, ItemType::Ems);
                emsItem->setText(0, formatEmsName(pack.Ems.back()));
                emsItem->setData(0, Qt::UserRole, pack.Ems.size() - 1);
            });
            menu.addAction("Add Pack", this, [this] {
                this->esl.Packs.emplace_back();
                const auto packItem = new QTreeWidgetItem(ui.treeWidgetPackSelector, ItemType::Pack);
                packItem->setText(0, QStringLiteral("Pack %1").arg(this->esl.Packs.size()));
                packItem->setData(0, Qt::UserRole, this->esl.Packs.size() - 1);
            });
            menu.addAction("Remove Pack", this, [this, packIndex] {
                const auto packIter = this->esl.Packs.begin() + packIndex;
                if (currentEms && currentEms >= packIter->Ems.data() && currentEms < packIter->Ems.data() + packIter->Ems.size())
                {
                    currentEms = nullptr;
                    loadEmsIntoUi({});
                }

                this->esl.Packs.erase(packIter);
                loadEslIntoUi();
            });
        }
        else if (item->type() == ItemType::Ems)
        {
            menu.addAction("Add Monster", this, [this, item] {
                auto& pack = this->esl.Packs[item->parent()->data(0, Qt::UserRole).toInt()];
                pack.Ems.emplace_back();
                const auto emsItem = new QTreeWidgetItem(item->parent(), ItemType::Ems);
                emsItem->setText(0, formatEmsName(pack.Ems.back()));
                emsItem->setData(0, Qt::UserRole, pack.Ems.size() - 1);
            });
            menu.addAction("Remove Monster", this, [this, item] {
                auto& pack = this->esl.Packs[item->parent()->data(0, Qt::UserRole).toInt()];
                const auto emsIndex = item->data(0, Qt::UserRole).toInt();
                if (currentEms == pack.Ems.data() + emsIndex)
                {
                    currentEms = nullptr;
                    loadEmsIntoUi({});
                }

                pack.Ems.erase(pack.Ems.begin() + emsIndex);
                loadEslIntoUi();
            });
        }
        else
        {
            return;
        }

        menu.addAction("Expand All", this, [this] {
            ui.treeWidgetPackSelector->expandAll();
        });
        menu.addAction("Collapse All", this, [this] {
            ui.treeWidgetPackSelector->collapseAll();
        });

        menu.exec(ui.treeWidgetPackSelector->mapToGlobal(pos));
    });

    connect(ui.treeWidgetPackSelector, &QTreeWidget::itemClicked, this, [this](const QTreeWidgetItem* item) {
        if (item->type() == ItemType::Ems)
        {
            const auto packIndex = item->parent()->data(0, Qt::UserRole).toInt();
            const auto emsIndex = item->data(0, Qt::UserRole).toInt();
            currentEms = this->esl.Packs[packIndex].Ems.data() + emsIndex;
            loadEmsIntoUi(*currentEms);
        }
    });
}

void EmSetListEditor::loadEmsIntoUi(const Resources::Ems& ems) const
{
    ui.comboMonster->setCurrentIndex(ui.comboMonster->findData(ems.MonsterId));
    ui.spinBoxSpawnCond->setValue(ems.SpawnCondition);
    ui.spinBoxArea->setValue(ems.Area);
    ui.doubleSpinBoxX->setValue(ems.Pos[0]);
    ui.doubleSpinBoxY->setValue(ems.Pos[1]);
    ui.doubleSpinBoxZ->setValue(ems.Pos[2]);
    ui.doubleSpinBoxAngle->setValue(ems.Angle);
    ui.spinBoxUnk0->setValue(ems.Unk1[0]);
    ui.spinBoxUnk1->setValue(ems.Unk1[1]);
    ui.spinBoxUnk2->setValue(ems.Unk2[0]);
    ui.spinBoxUnk3->setValue(ems.Unk2[1]);
    ui.spinBoxUnk4->setValue(ems.Unk3[0]);
    ui.spinBoxUnk5->setValue(ems.Unk3[1]);
}

Resources::Ems EmSetListEditor::saveEmsFromUi() const
{
    Resources::Ems ems;

    ems.MonsterId = ui.comboMonster->currentData().toInt();
    ems.SpawnCondition = ui.spinBoxSpawnCond->value();
    ems.Area = ui.spinBoxArea->value();
    ems.Pos[0] = (float)ui.doubleSpinBoxX->value();
    ems.Pos[1] = (float)ui.doubleSpinBoxY->value();
    ems.Pos[2] = (float)ui.doubleSpinBoxZ->value();
    ems.Angle = (float)ui.doubleSpinBoxAngle->value();
    ems.Unk1[0] = ui.spinBoxUnk0->value();
    ems.Unk1[1] = ui.spinBoxUnk1->value();
    ems.Unk2[0] = ui.spinBoxUnk2->value();
    ems.Unk2[1] = ui.spinBoxUnk3->value();
    ems.Unk3[0] = ui.spinBoxUnk4->value();
    ems.Unk3[1] = ui.spinBoxUnk5->value();

    return ems;
}

void EmSetListEditor::loadEslIntoUi()
{
    ui.treeWidgetPackSelector->clear();

    for (const auto [i, pack] : std::views::enumerate(esl.Packs))
    {
        const auto packItem = new QTreeWidgetItem(ui.treeWidgetPackSelector, ItemType::Pack);
        packItem->setText(0, QStringLiteral("Pack %1").arg(i + 1));
        packItem->setData(0, Qt::UserRole, QVariant::fromValue(i));
        for (const auto [j, ems] : std::views::enumerate(pack.Ems))
        {
            const auto emsItem = new QTreeWidgetItem(packItem, ItemType::Ems);
            emsItem->setText(0, formatEmsName(ems));
            emsItem->setData(0, Qt::UserRole, QVariant::fromValue(j));
        }
    }

    ui.treeWidgetPackSelector->expandAll();
}

QString EmSetListEditor::formatEmsName(const Resources::Ems& ems)
{
    return QStringLiteral("Area %1: %2").arg(ems.Area).arg(monsterNames[ems.MonsterId]);
}

