#pragma once

#include <vector>

#include <Common.h>

#include <QByteArray>
#include <QString>
#include <span>


namespace Resources
{

#pragma pack(push, 1)

enum class Map : s8;
enum class QuestLevel : u8;
enum class QuestEnemyLevel : u8;
enum class QuestType : u8;
enum class QuestSubType : u8;
enum class QuestStartType : u8;
enum class QuestBgmType : u8;
enum class QuestRequirement : u8;
enum class QuestClearType : u8;
enum class QuestClearParam : u8;
enum class QuestCarveLevel : u8;
enum class QuestGatheringLevel : u8;
enum class QuestFishingLevel : u8;
enum class QuestIcon : u8;
struct Language;

struct QuestClearCondition
{
    QuestClearParam Param;
    union
    {
        struct
        {
            u8 Value0;
            u8 Value1;
        };
        u16 Value;
    };
    u16 Count;
};

struct QuestSupplies
{
    s8 SuppLabel;
    s8 SuppType;
    u16 SuppTarget;
    u16 SuppTargetCount;
};

struct QuestMonster
{
    u16 Id;
    u8 SubType;
    s8 AuraType; // TODO: Enum
    u8 RestoreAmount;
    u8 HealthTableIndex;
    u8 AttackTableIndex;
    u8 OtherTableIndex;
    u8 Difficulty;
    u16 Size;
    u8 SizeTableIndex;
    u8 StaminaTableIndex;
};

struct QuestEnemySet
{
    s8 SetType;
    u16 TargetId;
    u16 TargetCount;
};

struct QuestMonsterSpawn
{
    s8 SpawnType;
    u16 SpawnTargetType;
    u16 SpawnTargetCount;
};

struct QuestInfo
{
    u32 TypeHash;
    char File[16];
};

struct Language
{
    enum : s32
    {
        Eng = 0,
        Fre,
        Ger,
        Ita,
        Spa,
        ChT,
        ChS,
        Count
    };

    static QString toString(s32 language)
    {
        switch (language)
        {
        case Eng: return "eng";
        case Fre: return "fre";
        case Ger: return "ger";
        case Ita: return "ita";
        case Spa: return "spa";
        case ChT: return "chT";
        case ChS: return "chS";
        default: return "unknown";
        }
    }
};

class QuestData
{
public:
    static constexpr u32 Magic = 0x434B0000; // 203.0f

    s32 Index;
    s32 Id;
    QuestType Type;
    QuestSubType SubType;
    QuestLevel Level;
    QuestEnemyLevel EnemyLevel;
    Map Map;
    QuestStartType StartType;
    u8 QuestTime;
    u8 Faints;
    u8 ArenaEquipId;
    QuestBgmType BgmType;
    QuestRequirement Requirement1;
    QuestRequirement Requirement2;
    QuestRequirement ComboRequirement;
    QuestClearType ClearType;
    u8 GekitaiHp;
    QuestClearCondition ClearConditions[2];
    QuestClearCondition SubClearCondition;
    QuestCarveLevel CarveLevel;
    QuestGatheringLevel GatheringLevel;
    QuestFishingLevel FishingLevel;
    s32 Fee;
    s32 VillagePoints;
    s32 Reward;
    s32 SubReward;
    s32 ClearVillagePoints;
    s32 FailVillagePoints;
    s32 SubVillagePoints;
    s32 HunterRankPoints;
    s32 SubHunterRankPoints;
    s8 RemAddFrame[2];
    s8 RemAddLotMax;
    QuestSupplies Supplies[2];
    QuestMonster Monsters[5];
    u8 SmallMonsterHpIndex;
    u8 SmallMonsterAtkIndex;
    u8 SmallMonsterOtherIndex;
    QuestEnemySet EnemySet2;
    QuestEnemySet EnemySet3;
    s8 BossRushType;
    QuestMonsterSpawn MonsterSpawns[5];
    u8 StrayRand;
    u8 StrayStartTime;
    u8 StrayStartRand;
    s8 StrayLimit345[3];
    u8 StrayRand345[3];
    u8 ExtraTicketCount;
    QuestIcon Icons[5];
    u32 ProgNum;
    QuestInfo Info[Language::Count];
    s32 VillagePointsG;
    u16 Flags;

    static QuestData deserialize(const QByteArray& data);
    static QuestData deserialize(std::span<const u8> data);
    static QByteArray serialize(const QuestData& quest);
};

enum class Map : s8
{
    JurassicFrontier_Day = 1,
    VerdantHills_Day = 2,
    ArcticRidge_Day = 3,
    MistyPeaks_Day = 4,
    Dunes,
    DesertedIsland,
    Marshlands,
    Volcano,
    Arena,
    V_Slayground,
    AncestralSteppe,
    VolcanicHollow,
    PrimalForest,
    FrozenSeaway,
    F_Slayground,
    Sanctuary,
    ForlornArena,
    S_Pinnacle,
    IngleIsle,
    PolarField,
    WyvernsEnd,
    Desert,
    Jungle,
    RuinedPinnacle,
    CastleSchrade,
    FortressRuins,
    ForlornCitadel,

    JurassicFrontier_Night = JurassicFrontier_Day + 100,
    VerdantHills_Night = VerdantHills_Day + 100,
    ArcticRidge_Night = ArcticRidge_Day + 100,
    MistyPeaks_Night = MistyPeaks_Day + 100,
};

enum class QuestLevel : u8
{
    Training = 0,
    Star1,
    Star2,
    Star3,
    Star4,
    Star5,
    Star6,
    Star7,
    Star8,
    Star9,
    Star10,
    G1,
    G2,
    G3,
    G4,
    G5,
    EX,

    Deviant1 = 100 + Star1,
    Deviant2 = 100 + Star2,
    Deviant3 = 100 + Star3,
    Deviant4 = 100 + Star4,
    Deviant5 = 100 + Star5,
    Deviant6 = 100 + Star6,
    Deviant7 = 100 + Star7,
    Deviant8 = 100 + Star8,
    Deviant9 = 100 + Star9,
    Deviant10 = 100 + Star10,
    DeviantG1 = 100 + G1,
    DeviantG2 = 100 + G2,
    DeviantG3 = 100 + G3,
    DeviantG4 = 100 + G4,
    DeviantG5 = 100 + G5,
    DeviantEX = 100 + EX,
};

enum class QuestEnemyLevel : u8
{
    Training = 0,
    LowRank = 0x1,
    HighRank = 0x3,
    GRank = 0x5,
};

enum class QuestType : u8
{
    Hunting = 0,
    Slaying,
    Capture,
    Gathering,
    HuntAThon,
    HuntAThonArena
};

enum class QuestSubType : u8
{
    Training = 0,
    Kokoto_Unlockable,
    Pokke_Unlockable,
    Yukumo_Unlockable,
    Bherna_Unlockable,

    Kokoto_Main,
    Pokke_Main,
    Yukumo_Main,
    Bherna_Main,

    ProwlerOnly,
    Default,
    SpecialPermit,
};

enum class QuestStartType : u8
{
    Camp = 0,
    Random,
    ElderDragon,
};

enum class QuestBgmType : u8
{
    Default = 0,
    ProwlerSpecial,
    Training,
};

enum class QuestRequirement : u8
{
    None = 0,
    HR2,
    HR3,
    HR4,
    HR5,
    HR6,
    HR7,
    HR8,
    HR9,
    HR10,
    HR11,
    HR12,
    HR13,
    HR20,
    HR25,
    HR30,
    HR45,
    HR50,
    HR60,
    HR100,
    GreatSwordOnly,
    LongSwordOnly,
    SwordAndShieldOnly,
    DualBladesOnly,
    LanceOnly,
    GunlanceOnly,
    HammerOnly,
    HuntingHornOnly,
    SwitchAxeOnly,
    ChargeBladeOnly,
    InsectGlaiveOnly,
    LightBowgunOnly,
    HeavyBowgunOnly,
    BowOnly,
    BlademasterOnly,
    GunnerOnly,
    GuildStyleOnly,
    StrikerStyleOnly,
    AerialStyleOnly,
    AdeptStyleOnly,
    ValorStyleOnly,
    AlchemyStyleOnly,
    NoProwler,
    ProwlerOnly,
    Rare1WeaponsOnly,
    NoArmorAndTalisman,
    NoItems,
    OnePlayerMax,
    TwoPlayersMax,
    ThreePlayersMax,
};

enum class QuestClearType : u8
{
    OneTarget = 0,
    TwoTargets,
    OneTargetAndTicket,
};

enum class QuestClearParam : u8
{
    None = 0,
    Hunt = 1, // Slay for small monsters
    Capture = 2,
    HuntAllLargeMonsters = 3,
    SlayTotalOfTargets = 4,
    DeliverItem = 5,
    EarnWycademyPoints = 6,
    DeliverPawPass = 7,
    BreakParts = 8,
    MountAndTopple = 9,
};

enum class QuestCarveLevel : u8
{
    LowRankSpecial = 0,
    LowRank,
    HighRank,
    Arena,
    GRank,
};

enum class QuestGatheringLevel : u8
{
    Arena0 = 0,
    LowRank,
    HighRank,
    Arena1,

    Special0,
    Special1,
    Special2,

    GRank = 9,
};

enum class QuestFishingLevel : u8
{
    LowRankSpecial = 0,
    LowRank,
    HighRank,
    Arena,
    GRank,
};

enum class QuestIcon : u8
{
    None = 0,
    Rathian,
    Gold_Rathian,
    Dreadqueen,
    Rathalos,
    Silver_Rathalos,
    Dreadking,
    Khezu,
    Yiak_Kut_Ku,
    Gypceros,
    Plesioth,
    Kirin,
    Velocidrome,
    Gendrome,
    Iodrome,
    Cephadrome,
    Yian_Garuga,
    Deadeye,
    Daimyo_Hermitaur,
    Stonefist,
    Shogun_Ceanataur,
    Blangonga,
    Rajang,
    Furious_Rajang,
    Kushala_Daora,
    Chameleos,
    Teostra,
    Bulldrome,
    Tigrex,
    Grimclaw,
    Akantor,
    Lavasioth,
    Nargacuga,
    Silverwind,
    Ukanlos,
    Deviljho,
    Savage_Deviljho,
    Uragaan,
    Crystalbeard,
    Lagiacrus,
    Royal_Ludroth,
    Agnaktor,
    Alatreon,
    Duramboros,
    Nibelsnarf,
    Zinogre,
    Thunderlord,
    Amatsu,
    Arzuros,
    Redhelm,
    Lagombi,
    Snowbaron,
    Volvidon,
    Brachydios,
    Kecha_Wacha,
    Tetsucabra,
    Drilltusk,
    Zamtrios,
    Najarala,
    Seltas_Queen,
    Gore_Magala,
    Shagaru_Magala,
    Seltas,
    Seregios,
    Malfestio,
    Glavenus,
    Hellblade,
    Astalos,
    Mizutsune,
    Gammoth,
    Nakarkos,
    Great_Maccao,
    Apnototh,
    Apceros,
    Kelbi,
    Mosswine,
    Hornetaur,
    Vespoid,
    Felyne,
    Melynx,
    Velociprey,
    Genprey,
    Ioprey,
    Cephalos,
    Bullfango,
    Popo,
    Giaprey,
    Anteka,
    Remobra,
    Hermitaur,
    Ceanataur,
    Blango,
    Rhenoplos,
    Bnahabra,
    Altaroth,
    Jaggi,
    Jaggia,
    Ludroth,
    Uroktor,
    Slagtoth,
    Gargwa,
    Zamite,
    Konchu,
    Maccao,
    Larinoth,
    Moofah,
    Danger,
    Cross_Symbol,
    Palico,
    Egg,
    Rocks,
    Fish,
    Bones,
    Insect,
    Mushroom,
    Accounting,
    Harvest_Tour,
    Box,
    Nakarkos_Head,
    Nakarkos_Tail,
    Basarios,
    Gravios,
    Diablos,
    Lao_Shan_Lung,
    Congalala,
    Giadrome,
    Barioth,
    Barroth,
    Raging_Brachydios,
    Nerscylla,
    Chaotic_Gore_Magala,
    Conga,
    Great_Thunderbug,
    Bloodbath,
    Boltreaver,
    Elderfrost,
    Soulseer,
    Rustrazor,
    Nightcloak,
    Ahtal_Ka_Mecha,
    Ahtal_Ka,
    Valstrax,
    None_Hyper,
    Rathian_Hyper,
    Gold_Rathian_Hyper,
    Rathalos_Hyper,
    Silver_Rathalos_Hyper,
    Khezu_Hyper,
    Basarios_Hyper,
    Gravios_Hyper,
    Diablos_Hyper,
    Yian_Kut_Ku_Hyper,
    Gypceros_Hyper,
    Plesioth_Hyper,
    Yian_Garuga_Hyper,
    Daimyo_Hermitaur_Hyper,
    Shogun_Ceanataur_Hyper,
    Congalala_Hyper,
    Blangonga_Hyper,
    Rajang_Hyper,
    Tigrex_Hyper,
    Lavasioth_Hyper,
    Nargacuga_Hyper,
    Barioth_Hyper,
    Deviljho_Hyper,
    Barroth_Hyper,
    Uragaan_Hyper,
    Lagiacrus_Hyper,
    Royal_Ludroth_Hyper,
    Agnaktor_Hyper,
    Duramboros_Hyper,
    Nibelsnarf_Hyper,
    Zinogre_Hyper,
    Brachydios_Hyper,
    Kecha_Wacha_Hyper,
    Tetsucabra_Hyper,
    Zamtrios_Hyper,
    Najarala_Hyper,
    Seltas_Queen_Hyper,
    Nerscylla_Hyper,
    Gore_Magala_Hyper,
    Seregios_Hyper,
    Malfestio_Hyper,
    Glavenus_Hyper,
    Astalos_Hyper,
    Mizutsune_Hyper,
    Gammoth_Hyper
};

#pragma pack(pop)

}
