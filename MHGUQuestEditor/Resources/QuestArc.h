#pragma once

#include <Common.h>
#include "Arc.h"
#include "QuestData.h"

namespace Resources
{

class QuestArc : public Arc
{
public:
    explicit QuestArc(std::filesystem::path path);

    const ArcEntry& getQuestData() const;
    ArcEntry& getQuestData();

    const ArcEntry* getGmd(s32 languageId, const QString& name) const;
    ArcEntry* getGmd(s32 languageId, const QString& name);

private:
    size_t questDataIndex;
};

}
