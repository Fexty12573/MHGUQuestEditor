#include "Arc.h"

#include <QtAssert>
#include <QtLogging>
#include <QDataStream>
#include <QFile>

#include <zlib.h>

struct ArcHeader
{
    u32 Magic;
    s16 Version;
    s16 FileCount;
    u32 Padding;
};

struct ArcEntryInternal
{
    char Path[64];
    u32 TypeHash;
    u32 CompSize;
    u32 RealSize : 29;
    u32 Quality : 3;
    u32 Offset;
};

Resources::Arc::Arc(std::filesystem::path path) : path(std::move(path))
{
    if (this->path.extension() != Arc::Extension)
    {
        qCritical("Invalid file extension %s", this->path.string().c_str());
        return;
    }

    load();
}

void Resources::Arc::load()
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qCritical("Failed to open file %s", path.string().c_str());
        return;
    }

    QDataStream stream(&file);
    if (stream.atEnd())
    {
        qCritical("File is empty %s", path.string().c_str());
        return;
    }

    ArcHeader header;
    stream.readRawData((char*)&header, sizeof(ArcHeader));

    if (header.Magic != Arc::Magic)
    {
        qCritical("Invalid magic %s", path.string().c_str());
        return;
    }

    if (header.Version != Arc::Version)
    {
        qCritical("Invalid version %s", path.string().c_str());
        return;
    }

    entries.reserve(header.FileCount);

    for (auto i = 0; i < header.FileCount; i++)
    {
        ArcEntryInternal entry;
        stream.readRawData((char*)&entry, sizeof(ArcEntryInternal));

        ArcEntry arcEntry = {
            .Path = QString::fromUtf8(entry.Path),
            .TypeHash = entry.TypeHash,
            .Extension = "", // TODO
            .CompSize = entry.CompSize,
            .RealSize = entry.RealSize,
            .Quality = entry.Quality
        };

        const auto pos = file.pos();
        file.seek(entry.Offset);
        arcEntry.Data.resize(entry.CompSize);
        stream.readRawData((char*)arcEntry.Data.data(), entry.CompSize);
        file.seek(pos);

        entries.emplace_back(arcEntry);
    }
}

void Resources::Arc::save(const std::filesystem::path& path)
{
    if (!path.empty())
    {
        this->path = path;
    }

    QFile file(this->path);
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical("Failed to open file %s", this->path.string().c_str());
        return;
    }

    QDataStream stream(&file);

    stream << Arc::Magic;
    stream << Arc::Version;
    stream << (s16)entries.size();
    stream << (u32)0;

    const auto fileTableLength = sizeof(ArcHeader) + entries.size() * sizeof(ArcEntryInternal);
    auto dataOffset = fileTableLength + (0x8000 - fileTableLength % 0x8000); // Align to 0x8000

    std::vector<qint64> offsets;

    for (const auto& entry : entries)
    {
        ArcEntryInternal internal = {
            .TypeHash = entry.TypeHash,
            .CompSize = entry.CompSize,
            .RealSize = entry.RealSize,
            .Quality = entry.Quality,
            .Offset = 0
        };

        const auto pathLen = std::min(sizeof(internal.Path), (size_t)entry.Path.toLatin1().size());
        std::memcpy(internal.Path, entry.Path.toUtf8().data(), pathLen);
        std::memset(internal.Path + pathLen, 0, sizeof(internal.Path) - pathLen);

        stream.writeRawData((const char*)&internal, sizeof(ArcEntryInternal));

        offsets.push_back(dataOffset);
        dataOffset += entry.CompSize;
    }

    // Pad to 0x8000
    const auto paddingSize = 0x8000 - file.pos() % 0x8000;
    const auto padding = new char[paddingSize];
    std::memset(padding, 0, paddingSize);
    stream.writeRawData(padding, paddingSize);

    for (auto i = 0; i < entries.size(); i++)
    {
        Q_ASSERT(file.pos() == offsets[i]);
        stream.writeRawData((const char*)entries[i].Data.data(), entries[i].CompSize);
    }
}

std::vector<u8> Resources::ArcEntry::getData(bool decompress) const
{
    if (!decompress)
    {
        return Data;
    }

    uLongf realSize = RealSize;
    std::vector<u8> decompressed(RealSize);

    if (uncompress(decompressed.data(), &realSize, Data.data(), (uLong)Data.size()) != Z_OK)
    {
        qCritical("Failed to decompress data");
        return {};
    }

    if (realSize != RealSize)
    {
        qCritical("Decompressed size mismatch");
        return {};
    }

    return decompressed;
}

void Resources::ArcEntry::setData(const std::vector<u8>& data, bool compress)
{
    if (!compress)
    {
        Data = data;
        CompSize = RealSize = (u32)data.size();
        return;
    }

    uLongf compSize = compressBound((uLong)data.size());
    std::vector<u8> compressed(compSize);

    if (::compress(compressed.data(), &compSize, data.data(), (uLong)data.size()) != Z_OK)
    {
        qCritical("Failed to compress data");
        return;
    }

    Data = compressed;
    Data.resize(compSize);
    CompSize = compSize;
    RealSize = (u32)data.size();
}
