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
        ArenaQuest q = quest;
        // Auto toggle for the charm slot (6th equipment piece):
        //   charm Id = charm type (0 = none, 2 = Dos charm, 10 = Ten charm);
        //   if any skill/points/decoration is set and the type is 0, enable the Dos charm;
        //   if all three are empty, disable it (Id = 0); an existing type is kept as is.
        for (auto& set : q.EquipSets) {
            auto& charm = set.Equipment[5];
            const bool hasSkill = set.CharmSkill1 != 0 || set.CharmSkill1Level != 0
                               || set.CharmSkill2 != 0 || set.CharmSkill2Level != 0;
            const bool hasDeco = charm.Decos[0] != 0 || charm.Decos[1] != 0 || charm.Decos[2] != 0;
            if (hasSkill || hasDeco) {
                if (charm.Id == 0)
                    charm.Id = 2; // Dos charm
            } else {
                charm.Id = 0;
            }
        }
        stream.writeRawData((const char*)&q, sizeof(ArenaQuest));
    }

    return data;
}
