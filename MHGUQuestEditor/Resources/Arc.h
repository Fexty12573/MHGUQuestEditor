#pragma once

#include <Common.h>

#include <QString>
#include <filesystem>
#include <optional>
#include <vector>
#include <span>


namespace Resources
{

struct ArcEntry
{
    QString Path;
    u32 TypeHash;
    QString Extension;
    u32 CompSize;
    u32 RealSize : 29;
    u32 Quality : 3;
    std::vector<u8> Data;

    std::vector<u8> getData(bool decompress = true) const;
    void setData(std::span<const u8> data, bool compress = true);
    void setData(const QByteArray& data, bool compress = true);
};

class Arc
{
public:
    static constexpr auto Extension = ".arc";
    static constexpr u32 Magic = 0x435241; // "ARC\0"
    static constexpr s16 Version = 0x0011;

private:
    static constexpr u32 DataAlignment = 0x10;

protected:
    std::filesystem::path path;
    std::vector<ArcEntry> entries;

    void load();
    
public:
    explicit Arc(std::filesystem::path path);

    std::span<const ArcEntry> getEntries() const;
    std::vector<ArcEntry>& getEntries();

    const ArcEntry* findEntry(QStringView path) const;
    ArcEntry* findEntry(QStringView path);

    const ArcEntry& getEntry(int index) const;
    ArcEntry& getEntry(int index);

    ArcEntry& addEntry(const QString& fpath, const QString& typeName, std::span<const u8> data, bool compressed = false, u32 realSize = 0);

    void save(const std::filesystem::path& path = {});
};

}
