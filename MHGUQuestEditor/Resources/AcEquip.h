#pragma once

#include <Common.h>

#include <QByteArray>
#include <QString>
#include <span>


namespace Resources {

#pragma pack(push, 1)
struct EquipmentPiece {
    u16 Id;
    u8 Level;
    u16 Decos[3];
};

struct Item {
    u16 Id;
    u8 Count;
};

struct EquipSet {
    u8 WeaponType;
    u8 WeaponLevel;
    u16 WeaponId;
    u16 WeaponDecos[3];
    u8 CustomAttr;
    u8 Style;
    u16 HunterArts[3];
    u8 CharmSkill1;
    s8 CharmSkill1Level;
    u8 CharmSkill2;
    s8 CharmSkill2Level;
    u8 InsectId;
    u8 InsectLevel;
    u8 InsectPower;
    u8 InsectToughness;
    u8 InsectSpeed;
    u8 InsectFire;
    u8 InsectWater;
    u8 InsectThunder;
    u8 InsectIce;
    u8 InsectDragon;
    EquipmentPiece Equipment[6];
    Item Items[32];
};

struct ArenaQuest {
    u32 Id;
    u8 PlayerCount;
    u32 RankTimes[3];
    u32 RankPoints[3];
    EquipSet EquipSets[5];
    u8 HunterArtsX1Flag;
    u8 HunterArtsX2Flag;
    u8 HunterArtsX3Flag;
};

struct AcEquip {
    static constexpr u32 Version = 0x40000000; // 2.0

    std::vector<ArenaQuest> Quests;

    static std::shared_ptr<AcEquip> deserialize(const QByteArray& data);
    static std::shared_ptr<AcEquip> deserialize(std::span<const u8> data);
    static QByteArray serialize(const AcEquip& acEquip);

    enum WeaponType : int {
        GreatSword = 7,
        SwordAndShield = 8,
        Hammer = 9,
        Lance = 10,
        HeavyBowgun = 11,
        LightBowgun = 13,
        Longsword = 14,
        SwitchAxe = 15,
        Gunlance = 16,
        Bow = 17,
        DualBlades = 18,
        HuntingHorn = 19,
        InsectGlaive = 20,
        ChargeBlade = 21,
    };

    static inline const std::unordered_map<u8, QString> WeaponTypes = {
        { 7, "Great Sword" },
        { 8, "Sword and Shield" },
        { 9, "Hammer" },
        { 10, "Lance" },
        { 11, "Heavy Bowgun" },
        { 13, "Light Bowgun" },
        { 14, "Long Sword" },
        { 15, "Switch Axe" },
        { 16, "Gunlance" },
        { 17, "Bow" },
        { 18, "Dual Blades" },
        { 19, "Hunting Horn" },
        { 20, "Insect Glaive" },
        { 21, "Charge Blade" },
    };

    static inline const std::unordered_map<u8, QString> Styles = {
        { 0, "Guild" },
        { 1, "Striker" },
        { 2, "Aerial" },
        { 3, "Adept" },
        { 4, "Alchemy" },
        { 5, "Valor" },
    };
};

#pragma pack(pop)



}
