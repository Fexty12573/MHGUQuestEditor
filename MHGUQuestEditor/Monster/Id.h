
#include <Common.h>

namespace Monster {

enum SubId : short;
enum Id : short;

struct IdType
{
    u8 Id;
    u8 SubId;

    IdType& operator=(enum Id id) 
    {
        Id = id & 0xFF;
        SubId = id >> 8;
        return *this;
    }

    operator enum Id() const
    {
        return (enum Id)(Id | SubId);
    }
};

enum SubId : u16
{
    None = 0,
    Subspecies = 1 << 8,
    RareSpecies = 2 << 8,
    Apex = 3 << 8,
    Deviant = 4 << 8,
    Variant = 5 << 8,
};

enum Id : u16
{
    None = 0,
    Rathian = 1,
    GoldRathian = Rathian | RareSpecies,
    DreadqueenRathian = Rathian | Deviant,
    Rathalos = 2,
    SilverRathalos = Rathalos | RareSpecies,
    DreadkingRathalos = Rathalos | Deviant,
    Khezu = 3,
    Basarios = 4,
    Gravios = 5,
    Diablos = 7,
    BloodbathDiablos = Diablos | Deviant,
    YianKutKu = 8,
    Gypceros = 9,
    Plesioth = 10,
    Kirin = 11,
    LaoShanLung = 12,
    Fatalis = 13,
    CrimsonFatalis = Fatalis | Subspecies,
    WhiteFatalis = Fatalis | RareSpecies,
    Velocidrome = 14,
    Gendrome = 15,
    Iodrome = 16,
    Cephadrome = 17,
    YianGaruga = 18,
    DeadEyeYianGaruga = YianGaruga | Deviant,
    DaimyoHermitaur = 19,
    StonefistHermitaur = DaimyoHermitaur | Deviant,
    ShogunCeanataur = 20,
    RustrazorCeanataur = ShogunCeanataur | Deviant,
    Congalala = 21,
    Blangonga = 22,
    Rajang = 23,
    FuriousRajang = Rajang | Variant,
    KushalaDaora = 24,
    Chameleos = 25,
    Teostra = 27,
    Bulldrome = 30,
    Tigrex = 32,
    GrimclawTigrex = Tigrex | Deviant,
    Akantor = 33,
    Giadrome = 34,
    Lavasioth = 36,
    Nargacuga = 37,
    SilverwindNargacuga = Nargacuga | Deviant,
    Ukanlos = 38,
    Barioth = 42,
    Deviljho = 43,
    SavageDeviljho = Deviljho | Variant,
    Barroth = 44,
    Uragaan = 45,
    CrystalbeardUragaan = Uragaan | Deviant,
    Lagiacrus = 46,
    RoyalLudroth = 47,
    Agnaktor = 49,
    Alatreon = 50,
    Duramboros = 55,
    Nibelsnarf = 56,
    Zinogre = 57,
    ThunderlordZinogre = Zinogre | Deviant,
    Amatsu = 58,
    Arzuros = 60,
    RedhelmArzuros = Arzuros | Deviant,
    Lagombi = 61,
    SnowbaronLagombi = Lagombi | Deviant,
    Volvidon = 62,
    Brachydios = 63,
    RagingBrachydios = Brachydios | Variant,
    KechaWacha = 65,
    Tetsucabra = 66,
    DrilltuskTetsucabra = Tetsucabra | Deviant,
    Zamtrios = 67,
    Najarala = 68,
    SeltasQueen = 69,
    Nerscylla = 70,
    GoreMagala = 71,
    ChaoticGoreMagala = GoreMagala | Variant,
    ShagaruMagala = 72,
    Seltas = 76,
    Seregios = 77,
    Malfestio = 79,
    NightcloakMalfestio = Malfestio | Deviant,
    Glavenus = 80,
    HellbladeGlavenus = Glavenus | Deviant,
    Astalos = 81,
    BoltreaverAstalos = Astalos | Deviant,
    Mizutsune = 82,
    SoulseerMizutsune = Mizutsune | Deviant,
    Gammoth = 83,
    ElderfrostGammoth = Gammoth | Deviant,
    Nakarkos = 84,
    GreatMaccao = 85,
    Valstrax = 86,
    AhtalNeset = 87,
    AhtalKa = 88,
};

}
