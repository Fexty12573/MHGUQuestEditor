#include "EmSetList.h"

#include <QDataStream>
#include <QIODevice>

#include <ranges>


Resources::EmSetList Resources::EmSetList::deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    EslHeader eslHeader;
    stream.readRawData((char*)&eslHeader, sizeof(EslHeader));

    if (eslHeader.Magic != EslMagic)
    {
        qCritical("Invalid magic for ESL %08X", eslHeader.Magic);
        return {};
    }

    EmSetList esl;

    for (const auto offset : eslHeader.Offsets) {
        if (offset == 0)
            continue;

        ZakoPack pack;

        stream.device()->seek(offset);

        EsdHeader esdHeader;
        stream.readRawData((char*)&esdHeader, sizeof(EsdHeader));

        if (esdHeader.Magic != EsdMagic)
        {
            qWarning("Invalid magic for ESD %08X", esdHeader.Magic);
            continue;
        }

        if (esdHeader.EmsCount == 0)
        {
            qWarning("No Ems entries found");
            continue;
        }

        for (u16 i = 0; i < esdHeader.EmsCount; ++i)
        {
            Ems ems;
            stream.readRawData((char*)&ems, sizeof(Ems));
            pack.Ems.push_back(ems);
        }

        esl.Packs.push_back(pack);
    }
}

Resources::EmSetList Resources::EmSetList::deserialize(std::span<const u8> data)
{
    return deserialize(QByteArray((const char*)data.data(), data.size()));
}

QByteArray Resources::EmSetList::serialize(const EmSetList& esl)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    constexpr u32 offsetListOffset = offsetof(EslHeader, Offsets);
    constexpr EslHeader eslHeader = {
        .Magic = EslMagic,
        .Version = EslVersion,
        .Offsets = {}
    };
    constexpr size_t maxPacks = std::size(eslHeader.Offsets);

    stream.writeRawData((const char*)&eslHeader, sizeof(EslHeader));
    
    std::vector<u32> offsets;

    if (esl.Packs.size() > maxPacks)
        qWarning("Too many packs, only writing first %zu", maxPacks);

    for (const auto& pack : esl.Packs | std::views::take(maxPacks))
    {
        offsets.push_back((u32)stream.device()->pos());
        const EsdHeader esdHeader = {
            .Magic = EsdMagic,
            .Version = EsdVersion,
            .EmsCount = (u16)pack.Ems.size()
        };

        stream.writeRawData((const char*)&esdHeader, sizeof(EsdHeader));
        stream.writeRawData((const char*)pack.Ems.data(), pack.Ems.size() * sizeof(Ems));
    }

    Q_ASSERT(offsets.size() <= maxPacks);
    
    const auto offsetsPtr = (u32*)(data.data() + offsetListOffset);
    for (const auto [i, off] : std::views::enumerate(offsets))
    {
        offsetsPtr[i] = off;
    }

    return data;
}
