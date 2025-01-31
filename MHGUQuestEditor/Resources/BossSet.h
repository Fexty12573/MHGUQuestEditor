#pragma once

#include <Common.h>

#include <QByteArray>
#include <QString>
#include <span>


namespace Resources {

struct Spawn {
    u32 Round;
    u32 Area;
    float Angle;
    float X;
    float Y;
    float Z;

    [[nodiscard]] bool empty() const {
        return Round == 0 && Area == 0 && Angle == 0 && X == 0 && Y == 0 && Z == 0;
    }
};

struct BossSet {
    static constexpr u32 Version = 0x3F800000; // 1.0f

    static Spawn deserialize(const QByteArray& data);
    static Spawn deserialize(std::span<const u8> data);
    static QByteArray serialize(const Spawn& spawn);
};

}
