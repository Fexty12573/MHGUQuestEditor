#include "Rem.h"

#include <QDataStream>


size_t Resources::Rem::size() const
{
    if (Rewards[0].ItemId == 0 && Rewards[0].Amount == 0 && Rewards[0].Weight == 0)
        return 0;

    for (int i = 0; i < std::size(Rewards); ++i)
    {
        if (Rewards[i].ItemId == 0 && Rewards[i].Amount == 0 && Rewards[i].Weight == 255)
            return i + 1;
    }

    return std::size(Rewards);
}

void Resources::Rem::remove(size_t index)
{
    if (index >= std::size(Rewards))
    {
        qCritical("Invalid index %zu", index);
        return;
    }

    for (size_t i = index; i < std::size(Rewards) - 1; ++i)
    {
        Rewards[i] = Rewards[i + 1];
    }

    Rewards[std::size(Rewards) - 1] = {};
}

size_t Resources::Rem::add(const RewardEntry& entry)
{
    if (Rewards[0].ItemId == 0 && Rewards[0].Amount == 0 && Rewards[0].Weight == 0)
    {
        Rewards[0] = entry;
        Rewards[1] = { .Weight = 255 };
        return 0;
    }

    for (int i = 0; i < std::size(Rewards); ++i)
    {
        if (Rewards[i].ItemId == 0 && Rewards[i].Amount == 0 && Rewards[i].Weight == 255)
        {
            Rewards[i] = entry;
            if (i + 1 < std::size(Rewards))
                Rewards[i + 1] = { .Weight = 255 };
            return i;
        }
    }

    qCritical("No more space for rewards");
    return -1;
}


Resources::Rem Resources::Rem::deserialize(const QByteArray& data)
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

    s32 remCount;
    stream >> remCount;

    if (remCount > 1) 
    {
        qCritical("Multiple Rem per Resource is not supported");
        return {};
    }

    Rem rem;
    stream.readRawData((char*)&rem, sizeof(Rem));

    return rem;
}

Resources::Rem Resources::Rem::deserialize(std::span<const u8> data)
{
    return deserialize(QByteArray((const char*)data.data(), data.size()));
}

QByteArray Resources::Rem::serialize(const Rem& rem)
{
    QByteArray data;
    QDataStream stream(&data, QIODeviceBase::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << Version;
    stream << 1; // Rem count
    stream.writeRawData((const char*)&rem, sizeof(Rem));
    
    return data;
}
