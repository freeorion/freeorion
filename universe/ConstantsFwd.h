#ifndef _ConstantsFwd_h_
#define _ConstantsFwd_h_

inline constexpr int INVALID_DESIGN_ID = -1;
inline constexpr int INCOMPLETE_DESIGN_ID = -4;
inline constexpr int INVALID_OBJECT_ID = -1; // The ID number assigned to a UniverseObject upon construction; It is assigned an ID later when it is placed in the universe
inline constexpr int ALL_EMPIRES = -1;

// sentinel values returned by CurrentTurn().  Can't be an enum since
// CurrentGameTurn() needs to return an integer game turn number
inline constexpr int INVALID_GAME_TURN = -(2 << 15) + 1;
inline constexpr int BEFORE_FIRST_TURN = -(2 << 14);
inline constexpr int IMPOSSIBLY_LARGE_TURN = 2 << 15;

#endif
