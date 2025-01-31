#include "BossSet.h"

#include <QDataStream>


Resources::Spawn Resources::BossSet::deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    Spawn spawn;

    u32 version;
    int count;

    stream >> version;
    stream >> count;

    if (version != Version)
    {
        qCritical("Invalid version %08X", version);
        return {};
    }

    if (count > 1)
    {
        qCritical("Multiple Spawns per BossSet Resource is not supported");
        return {};
    }

    Spawn s;
    stream.readRawData((char*)&s, sizeof(Spawn));

    return s;
}

Resources::Spawn Resources::BossSet::deserialize(std::span<const u8> data)
{
    return deserialize(QByteArray((const char*)data.data(), data.size()));
}

QByteArray Resources::BossSet::serialize(const Spawn& spawn)
{
    QByteArray data;
    QDataStream stream(&data, QIODeviceBase::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << Version;
    stream << 1;
    stream.writeRawData((const char*)&spawn, sizeof(Spawn));

    return data;
}
