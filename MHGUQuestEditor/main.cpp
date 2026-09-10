#include "MHGUQuestEditor.h"
#include "Util/AppLang.h"

#include <QtWidgets/QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Language: English uses the source strings (no translator); Chinese installs zh_CN.qm.
    // The setting lives in MHGUQuestEditor.ini and is switched from the "Language" menu.
    AppLang::loadSetting();
    QTranslator translator;
    if (!AppLang::english() && translator.load(QStringLiteral(":/lang/zh_CN.qm")))
    {
        a.installTranslator(&translator);
    }

    MHGUQuestEditor w;
    w.show();
    return a.exec();
}
