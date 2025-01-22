#pragma once

#include <Common.h>


namespace QuestId
{

enum class HubArea {
    HuntersHub = 0,
    Bherna,
    Kokoto,
    Pokke,
    Yukumo,
    PalicoRanch,
    Soaratorium,
    HuntersPub,
};

// Quest ID Format: ABCDEFG (decimal)
#define GET_DIGIT_A(ID) (((ID) / 1000000) % 10)
#define GET_DIGIT_B(ID) (((ID) / 100000) % 10)
#define GET_DIGIT_C(ID) (((ID) / 10000) % 10)
#define GET_DIGIT_DE(ID) (((ID) / 100) - ((ID) / 10000) * 100)
#define GET_DIGIT_FG(ID) ((ID) % 100)

inline bool isValid(u32 id, HubArea area) {
    constexpr auto isInRange = [](u32 value, u32 min, u32 max) {
        return min <= value && value <= max;
    };

    const auto a = GET_DIGIT_A(id);
    const auto c = GET_DIGIT_C(id);
    const auto de = GET_DIGIT_DE(id);
    const auto fg = GET_DIGIT_FG(id);


    switch (area) {
    case HubArea::HuntersHub:

    case HubArea::Bherna: [[fallthrough]];
    case HubArea::Kokoto: [[fallthrough]];
    case HubArea::Pokke: [[fallthrough]];
    case HubArea::Yukumo: [[fallthrough]];
    case HubArea::Soaratorium:
        switch (c) {
        case 0:

        case 3:
        default: return false;
        }

    case HubArea::HuntersPub:
        switch (c) {
        case 2: 
            return a != 1;

        case 1:
            if ((isInRange(de, 1, 7) || isInRange(de, 12, 13)) && isInRange(fg, 1, 99)) {
                return a != 1;
            }
            return false;

        case 4:
            if (isInRange(de, 1, 18) && isInRange(fg, 1, 16)) {
                return a != 1;
            }
            return false;

        default: 
            return false;
        }

    case HubArea::PalicoRanch: [[fallthrough]];
    default: return false;
    }
}

#undef GET_DIGIT_A
#undef GET_DIGIT_B
#undef GET_DIGIT_C
#undef GET_DIGIT_DE
#undef GET_DIGIT_FG

} // namespace QuestId
