#ifndef _ConstantsFwd_h_
#define _ConstantsFwd_h_

constexpr int INVALID_DESIGN_ID = -1;
constexpr int INCOMPLETE_DESIGN_ID = -4;
constexpr int INVALID_OBJECT_ID = -1; // The ID number assigned to a UniverseObject upon construction; It is assigned an ID later when it is placed in the universe
constexpr int ALL_EMPIRES = -1;

// sentinel values returned by CurrentTurn().  Can't be an enum since
// CurrentGameTurn() needs to return an integer game turn number
constexpr int INVALID_GAME_TURN = -(2 << 15) + 1;
constexpr int BEFORE_FIRST_TURN = -(2 << 14);
constexpr int IMPOSSIBLY_LARGE_TURN = 2 << 15;

#endif
