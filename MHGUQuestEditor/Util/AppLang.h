#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QString>

// Application language (English/Chinese) global state:
//   English by default; the choice is stored in MHGUQuestEditor.ini in the app directory.
//   - UI text: English = source strings (no translator); Chinese = install zh_CN.qm.
//   - Data resources (json/text tables): English reads :/res/, Chinese reads :/res_zh/.
namespace AppLang
{
    inline bool& englishRef()
    {
        static bool english = true;
        return english;
    }

    inline bool english()
    {
        return englishRef();
    }

    inline QString settingsPath()
    {
        return QCoreApplication::applicationDirPath() + QStringLiteral("/MHGUQuestEditor.ini");
    }

    inline void loadSetting()
    {
        QSettings s(settingsPath(), QSettings::IniFormat);
        englishRef() = s.value(QStringLiteral("language"), QStringLiteral("en")).toString() != QLatin1String("zh");
    }

    inline void saveSetting()
    {
        QSettings s(settingsPath(), QSettings::IniFormat);
        s.setValue(QStringLiteral("language"), english() ? QStringLiteral("en") : QStringLiteral("zh"));
    }

    // Pick the data resource path for the current language (icons/nan stay under :/res/)
    inline QString resFile(const char* name)
    {
        // English = upstream data (:/res/); Chinese = translated pack (:/res_zh/); other resources shared
        return (english() ? QStringLiteral(":/res/") : QStringLiteral(":/res_zh/")) + QLatin1String(name);
    }
}
