#pragma once

#include <Common.h>
#include "Arc.h"
#include "QuestData.h"

namespace Resources
{

class QuestArc : public Arc
{
public:
    explicit QuestArc(std::filesystem::path path, bool isRegularQuestArc = true);

    const ArcEntry& getQuestData() const;
    ArcEntry& getQuestData();

    const ArcEntry& getQuestLink() const;
    ArcEntry& getQuestLink();

    const ArcEntry* getQuestData(u32 questId) const;
    ArcEntry* getQuestData(u32 questId);

    bool addQuestData(u32 questId, const QByteArray& data, bool compressed = false, u32 realSize = 0);
    bool addQuestData(u32 questId, std::span<const u8> data, bool compressed = false, u32 realSize = 0);
    bool addGmd(s32 languageId, const QString& name, const QByteArray& data, bool compressed = false, u32 realSize = 0);
    bool addGmd(s32 languageId, const QString& name, std::span<const u8> data, bool compressed = false, u32 realSize = 0);
    bool addSem(u32 mapId, u32 emId, u32 semId, const QByteArray& data, bool compressed = false, u32 realSize = 0);
    bool addSem(u32 mapId, u32 emId, u32 semId, std::span<const u8> data, bool compressed = false, u32 realSize = 0);
    bool addRem(u32 remId, const QByteArray& data, bool compressed = false, u32 realSize = 0);
    bool addRem(u32 remId, std::span<const u8> data, bool compressed = false, u32 realSize = 0);
    bool addEsl(u32 mapId, u32 eslId, const QByteArray& data, bool compressed = false, u32 realSize = 0);
    bool addEsl(u32 mapId, u32 eslId, std::span<const u8> data, bool compressed = false, u32 realSize = 0);

    const ArcEntry* getGmd(s32 languageId, const QString& name) const;
    ArcEntry* getGmd(s32 languageId, const QString& name);

    void save(const std::filesystem::path& path = {}) override;

private:
    void fixOrder();

private:
    size_t questDataIndex;
    size_t questLinkIndex;
    bool isRegularQuestArc;
};

}
