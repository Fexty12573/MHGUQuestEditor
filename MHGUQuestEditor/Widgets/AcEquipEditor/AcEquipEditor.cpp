#include "AcEquipEditor.h"
#include <QListWidgetItem>
#include <QMenu>

#include <ranges>


class QuestListWidgetItem final : public QListWidgetItem
{
    int questIndex;

public:
    QuestListWidgetItem(const QString& text, int questIndex)
        : QListWidgetItem(text), questIndex(questIndex) { }

    int getQuestIndex() const { return questIndex; }
};

AcEquipEditor::AcEquipEditor(QWidget *parent)
    : QWidget(parent), acEquip(nullptr)
{
    ui.setupUi(this);

    equipSetEditors = {
        new EquipSetEditor(this),
        new EquipSetEditor(this),
        new EquipSetEditor(this),
        new EquipSetEditor(this),
        new EquipSetEditor(this)
    };

    for (int i = 0; i < equipSetEditors.size(); i++)
    {
        const auto widget = ui.tabWidgetEquipSets->widget(i);
        const auto layout = new QVBoxLayout(widget);
        layout->addWidget(equipSetEditors[i]);
        widget->setLayout(layout);
    }

    connect(ui.listWidgetQuests, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* current, QListWidgetItem* previous) {
        if (current)
        {
            const auto questItem = dynamic_cast<QuestListWidgetItem*>(current);
            if (questItem)
            {
                loadQuestData(questItem->getQuestIndex());
            }
        }
    });

    ui.listWidgetQuests->setCurrentItem(ui.listWidgetQuests->item(0));

    connect(ui.spinBoxQuestId, &QSpinBox::valueChanged, this, [this](int value) {
        auto& quest = this->acEquip->Quests[ui.listWidgetQuests->currentRow()];
        quest.Id = value;
        ui.listWidgetQuests->currentItem()->setText(QStringLiteral("q%1 - %2p").arg(value, 7, 10, QChar(u'0')).arg(quest.PlayerCount));
    });

    connect(ui.spinBoxPlayerCount, &QSpinBox::valueChanged, this, [this](int value) {
        auto& quest = this->acEquip->Quests[ui.listWidgetQuests->currentRow()];
        quest.PlayerCount = value;
        ui.listWidgetQuests->currentItem()->setText(QStringLiteral("q%1 - %2p").arg(quest.Id, 7, 10, QChar(u'0')).arg(value));
    });

    connect(ui.timeEditRankA, &QSpinBox::valueChanged, this, [this](int value) {
        auto& quest = this->acEquip->Quests[ui.listWidgetQuests->currentRow()];
        quest.RankTimes[0] = value;
    });
    connect(ui.timeEditRankB, &QSpinBox::valueChanged, this, [this](int value) {
        auto& quest = this->acEquip->Quests[ui.listWidgetQuests->currentRow()];
        quest.RankTimes[1] = value;
    });
    connect(ui.timeEditRankC, &QSpinBox::valueChanged, this, [this](int value) {
        auto& quest = this->acEquip->Quests[ui.listWidgetQuests->currentRow()];
        quest.RankTimes[2] = value;
    });
    connect(ui.spinBoxRankAPoints, &QSpinBox::valueChanged, this, [this](int value) {
        auto& quest = this->acEquip->Quests[ui.listWidgetQuests->currentRow()];
        quest.RankPoints[0] = value;
    });
    connect(ui.spinBoxRankBPoints, &QSpinBox::valueChanged, this, [this](int value) {
        auto& quest = this->acEquip->Quests[ui.listWidgetQuests->currentRow()];
        quest.RankPoints[1] = value;
    });
    connect(ui.spinBoxRankCPoints, &QSpinBox::valueChanged, this, [this](int value) {
        auto& quest = this->acEquip->Quests[ui.listWidgetQuests->currentRow()];
        quest.RankPoints[2] = value;
    });

    connect(ui.listWidgetQuests, &QListWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu menu;

        const auto item = ui.listWidgetQuests->itemAt(pos);
        if (!item || acEquip->Quests.size() == 1) return; // Don't allow deletion of the last quest

        menu.addAction("Delete Quest", this, [this, item] {
            const auto index = ui.listWidgetQuests->row(item);
            auto selectedIndex = ui.listWidgetQuests->currentRow();
            ui.listWidgetQuests->takeItem(index);
            acEquip->Quests.erase(acEquip->Quests.begin() + index);

            if (selectedIndex == index)
            {
                if (selectedIndex >= ui.listWidgetQuests->count())
                    selectedIndex = ui.listWidgetQuests->count() - 1;

                if (selectedIndex >= 0)
                {
                    ui.listWidgetQuests->setCurrentRow(selectedIndex);
                    loadQuestData(selectedIndex);
                }
                else 
                {
                    // Shouldn't happen
                    qFatal("No quests left");
                }
            }
        });
    });
}

void AcEquipEditor::loadAcEquip()
{
    for (const auto [i, quest] : std::views::enumerate(acEquip->Quests))
    {
        const auto questString = QStringLiteral("q%1 - %2p").arg(quest.Id, 7, 10, QChar(u'0')).arg(quest.PlayerCount);
        ui.listWidgetQuests->addItem(new QuestListWidgetItem(questString, (int)i));
    }

    ui.listWidgetQuests->setCurrentItem(ui.listWidgetQuests->item(0));

    if (!acEquip->Quests.empty())
    {
        loadQuestData(0);
    }
}

void AcEquipEditor::loadQuestData(int index)
{
    auto& quest = acEquip->Quests[index];

    ui.spinBoxQuestId->setValue(quest.Id);
    ui.spinBoxPlayerCount->setValue(quest.PlayerCount);
    ui.timeEditRankA->setValue(quest.RankTimes[0]);
    ui.timeEditRankB->setValue(quest.RankTimes[1]);
    ui.timeEditRankC->setValue(quest.RankTimes[2]);
    ui.spinBoxRankAPoints->setValue(quest.RankPoints[0]);
    ui.spinBoxRankBPoints->setValue(quest.RankPoints[1]);
    ui.spinBoxRankCPoints->setValue(quest.RankPoints[2]);

    for (auto [i, equipSet] : std::views::enumerate(quest.EquipSets))
    {
        ui.tabWidgetEquipSets->setTabText((int)i, QStringLiteral("%1 %2")
            .arg(Resources::AcEquip::Styles.at(equipSet.Style))
            .arg(Resources::AcEquip::WeaponTypes.at(equipSet.WeaponType))
        );

        equipSetEditors[i]->setEquipSet(&equipSet);
    }
}
