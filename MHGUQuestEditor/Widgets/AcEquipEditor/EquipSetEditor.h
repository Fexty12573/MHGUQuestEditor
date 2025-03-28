#pragma once

#include <QWidget>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QStandardItemModel>

#include "ui_EquipSetEditor.h"
#include "Resources/AcEquip.h"


struct ArmorData {
    int Id;
    int MaxLevel;
};

struct WeaponData {
    int Family;
    int Level;
};

class DecorationSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit DecorationSortFilterProxyModel(QStringListModel* sourceModel, QObject* parent = nullptr)
        : QSortFilterProxyModel(parent) {
        QSortFilterProxyModel::setSourceModel(sourceModel);
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
    {
        const auto itemText = sourceModel()->index(source_row, 0, source_parent).data().toString();

        return itemText.contains("Jwl", Qt::CaseInsensitive) || itemText == "None";
    }
};

class EquipSetEditor : public QWidget
{
    Q_OBJECT

public:
    EquipSetEditor(QWidget *parent = nullptr);
    ~EquipSetEditor() = default;

    void setEquipSet(Resources::EquipSet* set) {
        equipSet = set;
        loadEquipSet();
    }

private:
    void loadEquipSet();

private:
    Ui::EquipSetEditorClass ui;

    Resources::EquipSet* equipSet;

    static inline std::vector<ArmorData> ArmorSeriesData;
    static inline QStandardItemModel* HunterArtsModel;
    static inline QStandardItemModel* HeadArmorModel;
    static inline QStandardItemModel* ChestArmorModel;
    static inline QStandardItemModel* ArmArmorModel;
    static inline QStandardItemModel* WaistArmorModel;
    static inline QStandardItemModel* LegArmorModel;
    static inline QStringListModel* SkillNamesModel;
    static inline int ArmorComboMinWidth = 0;
    static inline std::array<QStandardItemModel*, 5> ArmorModels;
    static inline std::map<int, QStandardItemModel*> WeaponModels;
};
