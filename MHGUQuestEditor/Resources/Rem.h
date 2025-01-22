#pragma once

#include <Common.h>

#include <QByteArray>
#include <QString>
#include <span>

namespace Resources
{

#pragma pack(push, 1)

struct RewardEntry
{
    u16 ItemId = 0;
    u8 Amount = 0;
    u8 Weight = 0;
};

struct RemFlag
{
    u8 Flag;
    u8 Value;
};

class Rem
{
public:
    static constexpr u32 Version = 0x3F800000; // 1.0f

    RemFlag Flags[8];
    RewardEntry Rewards[40];

    size_t size() const;
    void remove(size_t index);
    size_t add(const RewardEntry& entry);

    static Rem deserialize(const QByteArray& data);
    static Rem deserialize(std::span<const u8> data);
    static QByteArray serialize(const Rem& rem);
};

#pragma pack(pop)

}
