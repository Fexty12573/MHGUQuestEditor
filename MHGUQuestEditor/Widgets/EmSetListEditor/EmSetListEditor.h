#pragma once

#include <QWidget>
#include "ui_EmSetListEditor.h"
#include "Resources/EmSetList.h"

#include <memory>
#include <unordered_map>


class EmSetListEditor final : public QWidget
{
    Q_OBJECT

public:
    explicit EmSetListEditor(Resources::EmSetList esl, QWidget *parent = nullptr);
    ~EmSetListEditor() override = default;

    void setEsl(const Resources::EmSetList& emSetList) {
        esl = emSetList;
        loadEslIntoUi();
    }
    [[nodiscard]] const Resources::EmSetList& getEsl() const { return esl; }

private:
    void loadEmsIntoUi(const Resources::Ems& ems) const;
    Resources::Ems saveEmsFromUi() const;
    void loadEslIntoUi();

    void importFromFile();

    static QString formatEmsName(const Resources::Ems& ems);
    
private:
    Ui::EmSetListEditorClass ui;
    Resources::EmSetList esl;
    Resources::Ems* currentEms = nullptr;
    Resources::Ems copiedEms;

    static std::unordered_map<int, QString> monsterNames;
};
