#include "MHGUQuestEditor.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>


MHGUQuestEditor::MHGUQuestEditor(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    QImage icons(":/res/cmn_micon00.png");
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

    QImage icons2(":/res/cmn_micon01.png");

    for (int y = 0; y < 14; y++)
    {
        for (int x = 0; x < 7; x++)
        {
            const int xPos = x * (iconSize + gap) + offsetX;
            const int yPos = y * (iconSize + gap) + offsetY;
            monsterIcons.emplace_back(QPixmap::fromImage(icons2.copy(xPos, yPos, iconSize, iconSize)));

            if (y == 6)
                break;
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

        if (i == 0)
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
}

MHGUQuestEditor::~MHGUQuestEditor() = default;
