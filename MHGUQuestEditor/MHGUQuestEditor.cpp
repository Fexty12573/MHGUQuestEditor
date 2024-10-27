#include "MHGUQuestEditor.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "Resources/StatTable.h"


MHGUQuestEditor::MHGUQuestEditor(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

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

    ui.labelIcon1->setPixmap(monsterIcons[1]);
    ui.labelIcon2->setPixmap(monsterIcons[21]);
    ui.labelIcon3->setPixmap(monsterIcons[111]);
    ui.labelIcon4->setPixmap(monsterIcons[61]);
    ui.labelIcon5->setPixmap(monsterIcons[106]);

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
    }

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

MHGUQuestEditor::~MHGUQuestEditor() = default;
