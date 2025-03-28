#pragma once

#include <QWidget>
#include "EquipSetEditor.h"
#include "ui_AcEquipEditor.h"
#include "Resources/AcEquip.h"

#include <array>
#include <memory>



class AcEquipEditor : public QWidget
{
    Q_OBJECT

public:
    AcEquipEditor(QWidget *parent = nullptr);
    ~AcEquipEditor() = default;

    void setAcEquip(const std::shared_ptr<Resources::AcEquip>& acEquip) {
        this->acEquip = acEquip;
        loadAcEquip();
    }
    [[nodiscard]] const std::shared_ptr<Resources::AcEquip>& getAcEquip() const { return acEquip; }

private:
    void loadAcEquip();
    void loadQuestData(int index);

private:
    Ui::AcEquipEditorClass ui;

    std::shared_ptr<Resources::AcEquip> acEquip;
    std::array<EquipSetEditor*, 5> equipSetEditors;
};
