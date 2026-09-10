#pragma once

#include <QAbstractItemView>
#include <QComboBox>
#include <QWidget>

// The popup widens to the longest entry so long text is not elided in a narrow window.
class StatCombo : public QComboBox
{
public:
    using QComboBox::QComboBox;

protected:
    void showPopup() override
    {
        QComboBox::showPopup();

        QAbstractItemView *viewItem = view();
        QWidget *popup = viewItem ? viewItem->window() : nullptr;
        if (!popup)
        {
            return;
        }

        const int content = viewItem->sizeHintForColumn(0);
        if (content > 0)
        {
            const int need = content + 46; // margin for frame and scrollbar
            if (popup->width() < need)
            {
                popup->resize(need, popup->height());
            }
        }
    }
};
