#include "AcEquip.h"

#include <QDataStream>


std::shared_ptr<Resources::AcEquip> Resources::AcEquip::deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    u32 version;
    stream >> version;

    if (version != Version) {
        qCritical("Invalid version %08X", version);
        return {};
    }

    s32 count;
    stream >> count;

    auto equip = std::make_shared<AcEquip>();
    for (s32 i = 0; i < count; i++) {
        auto& quest = equip->Quests.emplace_back();
        stream.readRawData((char*)&quest, sizeof(ArenaQuest));
    }

    return equip;
}

std::shared_ptr<Resources::AcEquip> Resources::AcEquip::deserialize(std::span<const u8> data)
{
    return deserialize(QByteArray((const char*)data.data(), data.size()));
}

QByteArray Resources::AcEquip::serialize(const AcEquip& acEquip)
{
    QByteArray data;
    QDataStream stream(&data, QIODeviceBase::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << Version;
    stream << (s32)acEquip.Quests.size();

    for (const auto& quest : acEquip.Quests) {
        stream.writeRawData((const char*)&quest, sizeof(ArenaQuest));
    }

    return data;
}
