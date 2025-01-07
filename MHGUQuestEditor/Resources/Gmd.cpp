#include "Gmd.h"

#include <QDataStream>


Resources::Gmd Resources::Gmd::deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    Gmd gmd;
    u32 magic;
    stream >> magic;
    if (magic != GmdHeader::Magic)
    {
        qCritical("Invalid magic %08X", magic);
        return {};
    }

    stream.readRawData((char*)&gmd.Header, sizeof(GmdHeader));

    gmd.PackageName.resize(gmd.Header.PackageNameLength + 1);
    stream.readRawData(gmd.PackageName.data(), gmd.Header.PackageNameLength + 1);

    for (u32 i = 0; i < gmd.Header.StringCount; ++i)
    {
        // Read null-terminated string
        std::string entry;
        char c;
        do
        {
            stream.readRawData(&c, 1);
            if (c != '\0')
                entry.push_back(c);
        } while (c != '\0');

        gmd.Entries.push_back(std::move(entry));
    }

    return gmd;
}

Resources::Gmd Resources::Gmd::deserialize(std::span<const u8> data)
{
    return deserialize(QByteArray((const char*)data.data(), data.size()));
}

QByteArray Resources::Gmd::serialize(const Gmd& gmd)
{
    QByteArray data;
    QDataStream stream(&data, QIODeviceBase::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    GmdHeader header = gmd.Header;
    header.PackageNameLength = (u32)gmd.PackageName.size();
    header.StringCount = (u32)gmd.Entries.size();
    header.UpdateTime = std::time(nullptr);

    stream << GmdHeader::Magic;
    stream.writeRawData((const char*)&header, sizeof(GmdHeader));
    stream.writeRawData(gmd.PackageName.c_str(), gmd.PackageName.size() + 1); // Include null terminator

    for (const auto& entry : gmd.Entries)
    {
        stream.writeRawData(entry.c_str(), entry.size() + 1); // Include null terminator
    }

    return data;
}
