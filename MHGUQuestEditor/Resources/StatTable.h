#pragma once

#include <Common.h>

#include <QByteArray>
#include <vector>

namespace Resources
{

struct StatTableEntry
{
    float Health;
    float Attack;
    float Defense;
    float Stagger;
    float Exhaust;
    float KO;
    float Mount;
    float Unknown;
};

class StatTable
{
public:
    static constexpr u32 Magic = 0x004E414E; // "NAN\0"
    static constexpr u32 Version = 0x14082501;

private:
    std::vector<StatTableEntry> entries;

public:
    size_t size() const
    {
        return entries.size();
    }

    const StatTableEntry& operator[](size_t index) const
    {
        return entries[index];
    }

    static StatTable deserialize(const QByteArray& data);
};

}
