#include "QuestLink.h"
#include "Util/Crc32.h"

#include <QDataStream>

Resources::QuestLink Resources::QuestLink::deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    u32 version;
    stream >> version;

    if (version != Version)
    {
        qCritical("Invalid version %08X", version);
        return {};
    }

    s32 count;
    stream >> count;

    if (count > 1)
    {
        qCritical("Multiple Quest Link per Resource is not supported");
        return {};
    }

    QuestLink link;
    stream.readRawData((char*)&link, sizeof(QuestLink));

    return link;
}

Resources::QuestLink Resources::QuestLink::deserialize(std::span<const u8> data)
{
    return deserialize(QByteArray((const char*)data.data(), data.size()));
}

QByteArray Resources::QuestLink::serialize(const QuestLink& link)
{
    QByteArray data;
    QDataStream stream(&data, QIODeviceBase::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << Version;
    stream << 1; // Quest Link count
    stream.writeRawData((const char*)&link, sizeof(QuestLink));

    return data;
}

void Resources::QuestLink::clearResource(LinkResource& resource)
{
    switch (resource.TypeHash)
    {
    case "rSetEmMain"_crc:
        std::memcpy(resource.File, EmptyBossSet.data(), EmptyBossSet.size());
        break;
    case "rEmSetList"_crc:
        std::memcpy(resource.File, EmptyEmSetList.data(), EmptyEmSetList.size());
        break;
    case "rRem"_crc:
        std::memcpy(resource.File, EmptyRem.data(), EmptyRem.size());
        break;
    default:
        qWarning("Invalid type hash: 0x%08X", resource.TypeHash);
        break;
    }
}

Resources::ResolvedLinkResource Resources::QuestLink::resolve(QuestArc& arc) const
{
    ResolvedLinkResource resolved{};

    resolved.BossSet[0] = resolveBossSet(arc, BossSet[0]);
    resolved.BossSet[1] = resolveBossSet(arc, BossSet[1]);
    resolved.BossSet[2] = resolveBossSet(arc, BossSet[2]);
    resolved.BossSet[3] = resolveBossSet(arc, BossSet[3]);
    resolved.BossSet[4] = resolveBossSet(arc, BossSet[4]);

    resolved.EmSetList[0] = resolveEmSetList(arc, EmSetList[0]);
    resolved.EmSetList[1] = resolveEmSetList(arc, EmSetList[1]);
    resolved.EmSetList[2] = resolveEmSetList(arc, EmSetList[2]);

    resolved.RemMain[0] = resolveRem(arc, RemMain[0]);
    resolved.RemMain[1] = resolveRem(arc, RemMain[1]);
    resolved.RemAdd[0] = resolveRem(arc, RemAdd[0]);
    resolved.RemAdd[1] = resolveRem(arc, RemAdd[1]);
    resolved.RemSub = resolveRem(arc, RemSub);

    if (Supp.TypeHash != 0)
    {
        const auto entry = arc.findEntry(QStringLiteral(R"(quest\supp\%1)").arg(Supp.File));
        if (!entry)
            qCritical("Failed to resolve Supp %08X %s", Supp.TypeHash, Supp.File);
        resolved.Supp = entry;
    }

    if (Plus.TypeHash != 0)
    {
        const auto entry = arc.findEntry(QStringLiteral(R"(quest\plus\questPlus_%1)").arg(Plus.File));
        if (!entry)
            qCritical("Failed to resolve Plus %08X %s", Plus.TypeHash, Plus.File);
        resolved.Plus = entry;
    }

    return resolved;
}

bool Resources::QuestLink::isEmptyResource(const LinkResource& resource)
{
    if (resource.TypeHash == 0)
        return true;

    switch (resource.TypeHash)
    {
    case "rSetEmMain"_crc:
        return resource.File == EmptyBossSet;
    case "rEmSetList"_crc:
        return resource.File == EmptyEmSetList;
    case "rRem"_crc:
        return resource.File == EmptyRem;
    default:
        qWarning("Unknown quest link type hash: 0x%08X", resource.TypeHash);
        return false;
    }
}

QString Resources::QuestLink::formatRemPath(const QString& remName)
{
    return QStringLiteral(R"(quest\rem\%1)").arg(remName);
}

QString Resources::QuestLink::formatRemPath(u32 remId)
{
    return QStringLiteral(R"(quest\rem\rem_%1)").arg(remId, 6, 10, QChar(u8'0'));
}

QString Resources::QuestLink::formatEslPath(const QString& eslName)
{
    return QStringLiteral(R"(quest\zako\emSetList\%1)").arg(eslName);
}

QString Resources::QuestLink::formatEslPath(u32 mapId, u32 eslId)
{
    return QStringLiteral(R"(quest\zako\emSetList\z_m%1d_%2)")
        .arg(mapId, 2, 10, QChar(u8'0'))
        .arg(eslId, 3, 10, QChar(u8'0'));
}

Resources::ArcEntry* Resources::QuestLink::resolveBossSet(QuestArc& arc, const LinkResource& resource)
{
    if (resource.TypeHash == 0)
        return nullptr;

    const auto entry = arc.findEntry(QStringLiteral(R"(quest\boss\setEmMain\%1)").arg(resource.File));
    if (!entry) {
        qCritical("Failed to resolve BossSet %08X %s", resource.TypeHash, resource.File);
        return nullptr;
    }

    // b_m00em000_00 is always an empty file and should not be resolved
    return resource.File == EmptyBossSet ? nullptr : entry;
}

Resources::ArcEntry* Resources::QuestLink::resolveEmSetList(QuestArc& arc, const LinkResource& resource)
{
    if (resource.TypeHash == 0)
        return nullptr;

    const auto entry = arc.findEntry(formatEslPath(resource.File));
    if (!entry) {
        qCritical("Failed to resolve EmSetList %08X %s", resource.TypeHash, resource.File);
        return nullptr;
    }

    // z_m00d_000 is always an empty file and should not be resolved
    return resource.File == EmptyEmSetList ? nullptr : entry;
}

Resources::ArcEntry* Resources::QuestLink::resolveRem(QuestArc& arc, const LinkResource& resource)
{
    if (resource.TypeHash == 0)
        return nullptr;

    const auto entry = arc.findEntry(formatRemPath(resource.File));
    if (!entry) {
        qCritical("Failed to resolve RemSub %08X %s", resource.TypeHash, resource.File);
        return nullptr;
    }

    // rem_000000 is always an empty file and should not be resolved
    return resource.File == EmptyRem ? nullptr : entry;
}
