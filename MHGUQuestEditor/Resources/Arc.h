#pragma once

#include <Common.h>

#include <filesystem>
#include <qstring.h>
#include <vector>


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
    void setData(const std::vector<u8>& data, bool compress = true);
};

class Arc
{
public:
    static constexpr auto Extension = ".arc";
    static constexpr u32 Magic = 0x435241; // "ARC\0"
    static constexpr s16 Version = 0x0011;

private:
    std::filesystem::path path;
    std::vector<ArcEntry> entries;

    void load();
    
public:
    explicit Arc(std::filesystem::path path);

    void save(const std::filesystem::path& path = {});
};

}
