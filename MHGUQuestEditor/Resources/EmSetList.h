#pragma once

#include <Common.h>

#include <QByteArray>
#include <QString>
#include <span>

namespace Resources
{

#pragma pack(push, 1)
struct Ems
{
    u16 MonsterId;
    u8 Unk1[2];
    s8 SpawnCondition;
    u8 Area;
    u16 Unk2[2];
    u16 Pad = 0xCDCD;
    float Pos[3];
    float Angle;
    u32 Unk3[2];
};

struct EslHeader
{
    u32 Magic;
    u32 Version;
    u32 Offsets[14];
};

struct EsdHeader
{
    u32 Magic;
    u32 Version;
    u16 EmsCount;
};
#pragma pack(pop)

struct ZakoPack
{
    std::vector<Ems> Ems;

    [[nodiscard]] bool empty() const;
};

class EmSetList
{
public:
    static constexpr u32 EslMagic = 0x6C7365; // "esl"
    static constexpr u32 EsdMagic = 0x445345; // "ESD"
    static constexpr u32 EslVersion = 2;
    static constexpr u32 EsdVersion = 0x20151214;

    std::vector<ZakoPack> Packs;

    [[nodiscard]] bool empty() const;

    static EmSetList deserialize(const QByteArray& data);
    static EmSetList deserialize(std::span<const u8> data);
    static QByteArray serialize(const EmSetList& esl);
};

}
