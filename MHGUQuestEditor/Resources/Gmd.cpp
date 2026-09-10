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
    header.StringCount = (u32)gmd.Entries.size();
    header.UpdateTime = std::time(nullptr);

    // Package name area: deserialize reads PackageNameLength + 1 bytes (incl. zero padding).
    // We must write back the same byte count and keep PackageNameLength in sync, otherwise
    // one extra byte is written per save and the text offset creeps forward (old bug: +1 per save).
    std::string packageName = gmd.PackageName;
    if (packageName.empty() || packageName.back() != '\0')
        packageName.push_back('\0');
    header.PackageNameLength = (u32)packageName.size() - 1;

    // String buffer size = total bytes of all entries including their trailing \0.
    // The old code kept the stale value; when text grew it was not updated and the game read garbage.
    u32 stringBufferSize = 0;
    for (const auto& entry : gmd.Entries)
        stringBufferSize += (u32)entry.size() + 1;
    header.StringBufferSize = stringBufferSize;

    stream << GmdHeader::Magic;
    stream.writeRawData((const char*)&header, sizeof(GmdHeader));
    stream.writeRawData(packageName.data(), (int)packageName.size());

    for (const auto& entry : gmd.Entries)
    {
        stream.writeRawData(entry.c_str(), entry.size() + 1); // Include null terminator
    }

    return data;
}
