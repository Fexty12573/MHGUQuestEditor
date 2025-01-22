#include "Arc.h"
#include "ExtensionResolver.h"
#include "Util/Crc32.h"

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

std::span<const Resources::ArcEntry> Resources::Arc::getEntries() const
{
    return entries;
}

std::vector<Resources::ArcEntry>& Resources::Arc::getEntries()
{
    return entries;
}

const Resources::ArcEntry* Resources::Arc::findEntry(QStringView path) const
{
    for (auto& entry : entries)
    {
        if (entry.Path == path)
        {
            return &entry;
        }
    }

    return nullptr;
}

Resources::ArcEntry* Resources::Arc::findEntry(QStringView path)
{
    for (auto& entry : entries)
    {
        if (entry.Path == path)
        {
            return &entry;
        }
    }

    return nullptr;
}

const Resources::ArcEntry& Resources::Arc::getEntry(int index) const
{
    if (index < 0 || index >= entries.size())
    {
        qFatal("Invalid index %d", index);
    }

    return entries[index];
}

Resources::ArcEntry& Resources::Arc::getEntry(int index)
{
    if (index < 0 || index >= entries.size())
    {
        qFatal("Invalid index %d", index);
    }

    return entries[index];
}

Resources::ArcEntry& Resources::Arc::addEntry(const QString& fpath, const QString& typeName, std::span<const u8> data, bool compressed, u32 realSize)
{
    const auto typeHash = type_hash(typeName.toLatin1());
    ArcEntry entry = {
        .Path = fpath,
        .TypeHash = typeHash,
        .Extension = ExtensionResolver::resolve(typeHash),
        .Quality = 2
    };

    entry.setData(data, !compressed);
    if (compressed && realSize != 0)
        entry.RealSize = realSize;

    entries.push_back(entry);
    return entries.back();
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
    stream.setByteOrder(QDataStream::LittleEndian);
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
            .Extension = ExtensionResolver::resolve(entry.TypeHash),
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
    stream.setByteOrder(QDataStream::LittleEndian);

    stream << Arc::Magic;
    stream << Arc::Version;
    stream << (s16)entries.size();
    stream << (u32)0;

    const auto fileTableLength = sizeof(ArcHeader) + entries.size() * sizeof(ArcEntryInternal);
    auto dataOffset = fileTableLength + (Arc::DataAlignment - fileTableLength % Arc::DataAlignment);

    std::vector<qint64> offsets;

    for (const auto& entry : entries)
    {
        ArcEntryInternal internal = {
            .TypeHash = entry.TypeHash,
            .CompSize = entry.CompSize,
            .RealSize = entry.RealSize,
            .Quality = entry.Quality,
            .Offset = (u32)dataOffset
        };

        const auto pathLen = std::min(sizeof(internal.Path), (size_t)entry.Path.toLatin1().size());
        std::memcpy(internal.Path, entry.Path.toUtf8().data(), pathLen);
        std::memset(internal.Path + pathLen, 0, sizeof(internal.Path) - pathLen);

        stream.writeRawData((const char*)&internal, sizeof(ArcEntryInternal));

        offsets.push_back(dataOffset);
        dataOffset += entry.CompSize;
    }

    // Pad to 0x8000
    const auto paddingSize = Arc::DataAlignment - file.pos() % Arc::DataAlignment;
    constexpr char padding[Arc::DataAlignment] = {};
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

void Resources::ArcEntry::setData(std::span<const u8> data, bool compress)
{
    if (!compress)
    {
        Data = { std::from_range, data };
        CompSize = (u32)data.size();
        return;
    }

    uLongf compSize = compressBound((uLong)data.size());
    Data.resize(compSize);

    if (::compress(Data.data(), &compSize, data.data(), (uLong)data.size()) != Z_OK)
    {
        qCritical("Failed to compress data");
        return;
    }

    Data.resize(compSize);
    CompSize = compSize;
    RealSize = (u32)data.size();
}

void Resources::ArcEntry::setData(const QByteArray& data, bool compress)
{
    setData({ (const u8*)data.data(), (size_t)data.size() }, compress);
}
