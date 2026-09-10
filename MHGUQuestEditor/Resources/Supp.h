#pragma once

#include <Common.h>

#include <QByteArray>
#include <span>


namespace Resources
{

#pragma pack(push, 1)

// Quest supply table (.sup): ver (1.0f) + u32 (always 1) + 40 entries of {itemId, count, flag}
// Layout verified against official samples and kiranico supply lists (first 32 + last 8 entries).
// Note: the 160 bytes after the 8-byte header are the 40 entries; the second u32 is not part of them.
struct SuppEntry
{
    u16 Id = 0;
    u8 Count = 0;
    u8 Flag = 0;
};

class Supp
{
public:
    static constexpr u32 Version = 0x3F800000; // 1.0f

    SuppEntry Entries[40];

    static Supp deserialize(const QByteArray& data);
    static Supp deserialize(std::span<const u8> data);
    static QByteArray serialize(const Supp& supp);
};

#pragma pack(pop)

}
