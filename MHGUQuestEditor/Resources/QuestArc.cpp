#include "QuestArc.h"

Resources::QuestArc::QuestArc(std::filesystem::path path)
    : Arc(std::move(path)), questDataIndex(-1)
{
    for (size_t i = 0; i < getEntries().size(); i++)
    {
        if (getEntries()[i].Extension == ".ext")
        {
            questDataIndex = i;
            return;
        }
    }

    if (questDataIndex == -1)
    {
        qFatal("Quest data not found in %s", this->path.string().c_str());
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

const Resources::ArcEntry* Resources::QuestArc::getGmd(s32 languageId, const QString& name) const
{
    return findEntry(Language::toString(languageId) + R"(\quest\questData\)" + name);
}

Resources::ArcEntry* Resources::QuestArc::getGmd(s32 languageId, const QString& name)
{
    return findEntry(Language::toString(languageId) + R"(\quest\questData\questData_)" + name);
}
