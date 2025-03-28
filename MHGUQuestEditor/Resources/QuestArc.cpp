#include "QuestArc.h"

#include "QuestLink.h"
#include "ExtensionResolver.h"
#include "Util/Crc32.h"

#include <ranges>

using namespace Qt::StringLiterals;

Resources::QuestArc::QuestArc(std::filesystem::path path, bool isRegularQuestArc)
    : Arc(std::move(path)), questDataIndex(-1), questLinkIndex(-1), isRegularQuestArc(isRegularQuestArc)
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

bool Resources::QuestArc::addQuestData(u32 questId, const QByteArray& data, bool compressed, u32 realSize)
{
    return addQuestData(questId, { (const u8*)data.data(), (size_t)data.size() }, compressed, realSize);
}

bool Resources::QuestArc::addQuestData(u32 questId, std::span<const u8> data, bool compressed, u32 realSize)
{
    const auto path = QStringLiteral(R"(loc\quest\questData\questData_%1)").arg(questId, 7, 10, QChar(u8'0'));
    if (findEntry(path))
    {
        qWarning("Duplicate quest data %s", path.toStdString().c_str());
        return false;
    }

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
    return true;
}

bool Resources::QuestArc::addGmd(s32 languageId, const QString& name, const QByteArray& data, bool compressed, u32 realSize)
{
    return addGmd(languageId, name, { (const u8*)data.data(), (size_t)data.size() }, compressed, realSize);
}

bool Resources::QuestArc::addGmd(s32 languageId, const QString& name, std::span<const u8> data, bool compressed, u32 realSize)
{
    const auto path = Language::toString(languageId) + R"(\quest\questData\questData_)" + name;
    if (findEntry(path))
    {
        qWarning("Duplicate GMD %s", path.toStdString().c_str());
        return false;
    }

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
    return true;
}

bool Resources::QuestArc::addSem(u32 mapId, u32 emId, u32 semId, const QByteArray& data, bool compressed, u32 realSize)
{
    return addSem(mapId, emId, semId, { (const u8*)data.data(), (size_t)data.size() }, compressed, realSize);
}

bool Resources::QuestArc::addSem(u32 mapId, u32 emId, u32 semId, std::span<const u8> data, bool compressed, u32 realSize)
{
    const auto path = QuestLink::formatBossSetPath(mapId, emId, semId);
    if (findEntry(path))
    {
        qWarning("Duplicate SEM %s", path.toStdString().c_str());
        return false;
    }

    ArcEntry entry = {
        .Path = path,
        .TypeHash = "rSetEmMain"_ext,
        .Extension = ExtensionResolver::resolve("rSetEmMain"_ext),
        .CompSize = 0,
        .RealSize = 0,
        .Quality = 2
    };

    entry.setData(data, !compressed);
    if (compressed && realSize != 0)
        entry.RealSize = realSize;

    entries.push_back(entry);
    return true;
}

bool Resources::QuestArc::addRem(u32 remId, const QByteArray& data, bool compressed, u32 realSize)
{
    return addRem(remId, { (const u8*)data.data(), (size_t)data.size() }, compressed, realSize);
}

bool Resources::QuestArc::addRem(u32 remId, std::span<const u8> data, bool compressed, u32 realSize)
{
    const auto path = QuestLink::formatRemPath(remId);
    if (findEntry(path))
    {
        qWarning("Duplicate REM %s", path.toStdString().c_str());
        return false;
    }

    ArcEntry entry = {
        .Path = path,
        .TypeHash = "rRem"_ext,
        .Extension = ExtensionResolver::resolve("rRem"_ext),
        .CompSize = 0,
        .RealSize = 0,
        .Quality = 2
    };

    entry.setData(data, !compressed);
    if (compressed && realSize != 0)
        entry.RealSize = realSize;

    entries.push_back(entry);
    return true;
}

bool Resources::QuestArc::addEsl(u32 mapId, u32 eslId, const QByteArray& data, bool compressed, u32 realSize)
{
    return addEsl(mapId, eslId, { (const u8*)data.data(), (size_t)data.size() }, compressed, realSize);
}

bool Resources::QuestArc::addEsl(u32 mapId, u32 eslId, std::span<const u8> data, bool compressed, u32 realSize)
{
    const auto path = QuestLink::formatEslPath(mapId, eslId);
    if (findEntry(path))
    {
        qWarning("Duplicate ESL %s", path.toStdString().c_str());
        return false;
    }

    ArcEntry entry = {
        .Path = path,
        .TypeHash = "rEmSetList"_ext,
        .Extension = ExtensionResolver::resolve("rEmSetList"_ext),
        .CompSize = 0,
        .RealSize = 0,
        .Quality = 2
    };

    entry.setData(data, !compressed);
    if (compressed && realSize != 0)
        entry.RealSize = realSize;

    entries.push_back(entry);
    return true;
}

const Resources::ArcEntry* Resources::QuestArc::getGmd(s32 languageId, const QString& name) const
{
    return findEntry(Language::toString(languageId) + R"(\quest\questData\)" + name);
}

Resources::ArcEntry* Resources::QuestArc::getGmd(s32 languageId, const QString& name)
{
    return findEntry(Language::toString(languageId) + R"(\quest\questData\questData_)" + name);
}

void Resources::QuestArc::save(const std::filesystem::path& path)
{
    if (isRegularQuestArc)
        fixOrder();

    Arc::save(path);
}

std::vector<const Resources::ArcEntry*> Resources::QuestArc::getSortedEntries() const
{
    if (!isRegularQuestArc)
        return Arc::getSortedEntries();

    // Quest Arc Order:
    // 5x SEM
    // 2x ESL
    // 5x REM
    // 1x SUPP
    // 1x PLUS
    // 1x Quest Link
    // 7x GMD
    // 1x Quest Data

    if (entries.size() < 22)
    {
        qWarning("Potentially missing quest arc entries (has %zu)", entries.size());
    }

    std::vector<const ArcEntry*> sorted;
    sorted.reserve(entries.size());

    const auto ext_filter = [](const char* ext) {
        return std::views::filter([ext](const auto& entry) { return entry.Extension == ext; });
    };

    for (const auto& sem : entries | ext_filter(".sem"))
        sorted.push_back(&sem);

    for (const auto& esl : entries | ext_filter(".esl"))
        sorted.push_back(&esl);

    for (const auto& rem : entries | ext_filter(".rem"))
        sorted.push_back(&rem);

    for (const auto& supp : entries | ext_filter(".sup"))
        sorted.push_back(&supp);

    for (const auto& plus : entries | ext_filter(".plus"))
        sorted.push_back(&plus);

    for (const auto& questLink : entries | ext_filter(".qdl"))
        sorted.push_back(&questLink);

    for (const auto& gmd : entries | ext_filter(".gmd"))
        sorted.push_back(&gmd);

    for (const auto& questData : entries | ext_filter(".ext"))
        sorted.push_back(&questData);

    return sorted;
}

void Resources::QuestArc::fixOrder()
{
    if (entries.size() < 22)
    {
        qCritical("Invalid quest arc entry count %zu", entries.size());
        return;
    }

    if (questDataIndex != entries.size() - 1)
    {
        std::swap(entries[questDataIndex], entries.back());
        questDataIndex = entries.size() - 1;
    }

    if (questLinkIndex != entries.size() - 9)
    {
        std::swap(entries[questLinkIndex], entries[entries.size() - 9]);
        questLinkIndex = entries.size() - 9;
    }
}
