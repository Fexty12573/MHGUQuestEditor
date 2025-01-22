#include "QuestArc.h"

#include "ExtensionResolver.h"
#include "Util/Crc32.h"

using namespace Qt::StringLiterals;

Resources::QuestArc::QuestArc(std::filesystem::path path)
    : Arc(std::move(path)), questDataIndex(-1), questLinkIndex(-1)
{
    for (size_t i = 0; i < getEntries().size(); i++)
    {
        if (entries[i].Extension == ".ext")
        {
            questDataIndex = i;
            continue;
        }

        if (entries[i].Extension == ".qdl")
        {
            questLinkIndex = i;
            continue;
        }
    }

    if (questDataIndex == -1)
    {
        qCritical("Quest data not found in %s", this->path.string().c_str());
    }

    if (questLinkIndex == -1)
    {
        qCritical("Quest link not found in %s", this->path.string().c_str());
    }
}

const Resources::ArcEntry& Resources::QuestArc::getQuestData() const
{
    return entries[questDataIndex];
}

Resources::ArcEntry& Resources::QuestArc::getQuestData()
{
    return entries[questDataIndex];
}

const Resources::ArcEntry& Resources::QuestArc::getQuestLink() const
{
    return entries[questLinkIndex];
}

Resources::ArcEntry& Resources::QuestArc::getQuestLink()
{
    return entries[questLinkIndex];
}

const Resources::ArcEntry* Resources::QuestArc::getQuestData(u32 questId) const
{
    return findEntry(QStringLiteral(R"(loc\quest\questData\questData_%1)").arg(questId, 7, 10, QChar(u8'0')));
}

Resources::ArcEntry* Resources::QuestArc::getQuestData(u32 questId)
{
    return findEntry(QStringLiteral(R"(loc\quest\questData\questData_%1)").arg(questId, 7, 10, QChar(u8'0')));
}

void Resources::QuestArc::addQuestData(u32 questId, const QByteArray& data, bool compressed, u32 realSize)
{
    addQuestData(questId, { (const u8*)data.data(), (size_t)data.size() }, compressed, realSize);
}

void Resources::QuestArc::addQuestData(u32 questId, std::span<const u8> data, bool compressed, u32 realSize)
{
    const auto path = QStringLiteral(R"(loc\quest\questData\questData_%1)").arg(questId, 7, 10, QChar(u8'0'));
    ArcEntry entry = {
        .Path = path,
        .TypeHash = "rQuestData"_ext,
        .Extension = ExtensionResolver::resolve("rQuestData"_ext),
        .CompSize = 0,
        .RealSize = 0,
        .Quality = 2
    };

    entry.setData(data, !compressed);
    if (compressed && realSize != 0)
        entry.RealSize = realSize;

    entries.push_back(entry);
}

void Resources::QuestArc::addGmd(s32 languageId, const QString& name, const QByteArray& data, bool compressed, u32 realSize)
{
    addGmd(languageId, name, { (const u8*)data.data(), (size_t)data.size() }, compressed, realSize);
}

void Resources::QuestArc::addGmd(s32 languageId, const QString& name, std::span<const u8> data, bool compressed, u32 realSize)
{
    const auto path = Language::toString(languageId) + R"(\quest\questData\questData_)" + name;
    ArcEntry entry = {
        .Path = path,
        .TypeHash = "rGUIMessage"_ext,
        .Extension = ExtensionResolver::resolve("rGUIMessage"_ext),
        .CompSize = 0,
        .RealSize = 0,
        .Quality = 2
    };

    entry.setData(data, !compressed);
    if (compressed && realSize != 0)
        entry.RealSize = realSize;

    entries.push_back(entry);
}

const Resources::ArcEntry* Resources::QuestArc::getGmd(s32 languageId, const QString& name) const
{
    return findEntry(Language::toString(languageId) + R"(\quest\questData\)" + name);
}

Resources::ArcEntry* Resources::QuestArc::getGmd(s32 languageId, const QString& name)
{
    return findEntry(Language::toString(languageId) + R"(\quest\questData\questData_)" + name);
}
