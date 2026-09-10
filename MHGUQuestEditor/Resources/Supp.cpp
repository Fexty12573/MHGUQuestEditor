#include "Supp.h"

#include <QDataStream>


Resources::Supp Resources::Supp::deserialize(const QByteArray& data)
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
        qCritical("Multiple Supp per Resource is not supported");
        return {};
    }

    Supp supp;
    stream.readRawData((char*)&supp, sizeof(Supp));

    return supp;
}

Resources::Supp Resources::Supp::deserialize(std::span<const u8> data)
{
    return deserialize(QByteArray((const char*)data.data(), data.size()));
}

QByteArray Resources::Supp::serialize(const Supp& supp)
{
    QByteArray data;
    QDataStream stream(&data, QIODeviceBase::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << Version;
    stream << 1; // Supp count
    stream.writeRawData((const char*)&supp, sizeof(Supp));

    return data;
}
