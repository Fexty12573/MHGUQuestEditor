#include "BossSetEditor.h"

BossSetEditor::BossSetEditor(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.spinBoxArea, &QSpinBox::valueChanged, this, [this](int value) { spawn.Area = value; });
    connect(ui.spinBoxRound, &QSpinBox::valueChanged, this, [this](int value) { spawn.Round = value; });
    connect(ui.doubleSpinBoxAngle, &QDoubleSpinBox::valueChanged, this, [this](double value) { spawn.Angle = (float)value; });
    connect(ui.doubleSpinBoxX, &QDoubleSpinBox::valueChanged, this, [this](double value) { spawn.X = (float)value; });
    connect(ui.doubleSpinBoxY, &QDoubleSpinBox::valueChanged, this, [this](double value) { spawn.Y = (float)value; });
    connect(ui.doubleSpinBoxZ, &QDoubleSpinBox::valueChanged, this, [this](double value) { spawn.Z = (float)value; });
}

BossSetEditor::~BossSetEditor() = default;

void BossSetEditor::setSpawn(const Resources::Spawn& mySpawn)
{
    spawn = mySpawn;
    loadSpawnIntoUi();
}

void BossSetEditor::loadSpawnIntoUi()
{
    ui.spinBoxArea->setValue(spawn.Area);
    ui.spinBoxRound->setValue(spawn.Round);
    ui.doubleSpinBoxAngle->setValue(spawn.Angle);
    ui.doubleSpinBoxX->setValue(spawn.X);
    ui.doubleSpinBoxY->setValue(spawn.Y);
    ui.doubleSpinBoxZ->setValue(spawn.Z);
}
