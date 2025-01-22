#include "SettingsDialog.h"

#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *parent, QString questList, bool autoUpdate)
    : QDialog(parent), questListPath(std::move(questList)), autoUpdateQuestList(autoUpdate)
{
    ui.setupUi(this);

    ui.textQuestList->setText(questListPath);
    ui.checkBoxAutoUpdate->setChecked(autoUpdateQuestList);

    connect(ui.buttonBrowseQuestList, &QToolButton::clicked, this, &SettingsDialog::browseForQuestList);
    connect(ui.cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);
    connect(ui.okButton, &QPushButton::clicked, this, [this] {
        questListPath = ui.textQuestList->text();
        autoUpdateQuestList = ui.checkBoxAutoUpdate->isChecked();
        accept();
    });
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::browseForQuestList()
{
    const auto path = QFileDialog::getOpenFileName(this, "Select Quest List", {}, "Arc File (*.arc)");
    if (path.isNull())
        return;

    ui.textQuestList->setText(path);
}
