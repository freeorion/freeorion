#ifndef _ConstantsFwd_h_
#define _ConstantsFwd_h_

#include "../util/StrongTypedef.h"
#include <cstdint>

inline constexpr int INVALID_DESIGN_ID = -1;
inline constexpr int INCOMPLETE_DESIGN_ID = -4;

//FO_STRONG_ID_TYPEDEF(EmpireID, int8_t)
//inline constexpr EmpireID ALL_EMPIRES{-1};

//FO_STRONG_ID_TYPEDEF(UniverseObjectID, int32_t)
//inline constexpr UniverseObjectID INVALID_OBJECT_ID{-1};

struct EID_tag {};
using EmpireID = StrongIDTypedef<EID_tag, int8_t, -1, -1>;
inline constexpr EmpireID ALL_EMPIRES = EmpireID::Invalid();

struct UID_tag {};
using UniverseObjectID = StrongIDTypedef<UID_tag, int32_t, -1, -1>;
inline constexpr UniverseObjectID INVALID_OBJECT_ID = UniverseObjectID::Invalid();


// sentinel values returned by CurrentTurn().  Can't be an enum since
// CurrentGameTurn() needs to return an integer game turn number
inline constexpr int INVALID_GAME_TURN = -(2 << 15) + 1;
inline constexpr int BEFORE_FIRST_TURN = -(2 << 14);
inline constexpr int IMPOSSIBLY_LARGE_TURN = 2 << 15;

#endif
