#pragma once

#include <QWidget>
#include "ui_BossSetEditor.h"

#include "Resources/BossSet.h"


class BossSetEditor : public QWidget
{
    Q_OBJECT

public:
    explicit BossSetEditor(QWidget *parent = nullptr);
    ~BossSetEditor() override;

    void setSpawn(const Resources::Spawn& mySpawn);
    const Resources::Spawn& getSpawn() const { return spawn; }

private:
    void loadSpawnIntoUi();

private:
    Ui::BossSetEditorClass ui;
    Resources::Spawn spawn;
};
