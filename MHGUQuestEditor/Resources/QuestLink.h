#pragma once

#include <Common.h>
#include "QuestArc.h"

#include <QByteArray>
#include <QString>
#include <span>
#include <string_view>


namespace Resources
{

#pragma pack(push, 1)

struct LinkResource
{
    u32 TypeHash;
    char File[16];
};

struct ResolvedLinkResource
{
    ArcEntry* BossSet[5];
    ArcEntry* EmSetList[3];
    ArcEntry* RemMain[2];
    ArcEntry* RemAdd[2];
    ArcEntry* RemSub;
    ArcEntry* Supp;
    ArcEntry* Plus;
};

class QuestLink
{
public:
    static constexpr u32 Version = 0x434B0000; // 203.0f
    static constexpr std::string_view EmptyBossSet = "b_m00em000_00";
    static constexpr std::string_view EmptyEmSetList = "z_m00d_000";
    static constexpr std::string_view EmptyRem = "rem_000000"; 

    LinkResource BossSet[5];
    LinkResource EmSetList[3];
    LinkResource RemMain[2];
    LinkResource RemAdd[2];
    LinkResource RemSub;
    LinkResource Supp;
    LinkResource Plus;

    static QuestLink deserialize(const QByteArray& data);
    static QuestLink deserialize(std::span<const u8> data);
    static QByteArray serialize(const QuestLink& link);

    static void clear_resource(LinkResource& resource);

    ResolvedLinkResource resolve(QuestArc& arc) const;

private:
    static ArcEntry* resolveBossSet(QuestArc& arc, const LinkResource& resource);
    static ArcEntry* resolveEmSetList(QuestArc& arc, const LinkResource& resource);
    static ArcEntry* resolveRem(QuestArc& arc, const LinkResource& resource);
};

#pragma pack(pop)

}
