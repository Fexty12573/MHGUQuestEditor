#pragma once

#include <Common.h>

#include <span>
#include <string>
#include <vector>

#include <QByteArray>


namespace Resources
{

#pragma pack(push, 1)

struct GmdHeader
{
    static constexpr u32 Magic = 0x444D47; // GMD\0

    u32 Version;
    s32 LanguageId;
    s64 UpdateTime;
    u32 IndexCount;
    u32 StringCount;
    u32 IndexNameBufferSize;
    u32 StringBufferSize;
    u32 PackageNameLength;
};

struct Gmd
{
    GmdHeader Header;
    std::string PackageName;
    std::vector<std::string> Entries;

    static Gmd deserialize(const QByteArray& data);
    static Gmd deserialize(std::span<const u8> data);
    static QByteArray serialize(const Gmd& gmd);
};

#pragma pack(pop)

}
