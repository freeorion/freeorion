// -*- C++ -*-
#ifndef _MultiplayerCommon_h_
#define _MultiplayerCommon_h_

#include "GGClr.h"
#include "XMLDoc.h"

#include <vector>

/** The colors that are available for use for empires in the game. */
const std::vector<GG::Clr>& EmpireColors();

/** Returns an MD5 "sum" of the given string as a 32-digithexidecimal string. */
std::string MD5StringSum(const std::string& str);

/** Returns an MD5 "sum" of the given text file as a 32-digit hexidecimal string. */
std::string MD5FileSum(const std::string& filename);

/** The data for one empire necessary for game-setup during multiplayer loading. */
struct SaveGameEmpireData
{
    /** \name Structors */ //@{
    SaveGameEmpireData(); ///< default ctor.
    SaveGameEmpireData(const GG::XMLElement& elem); ///< GG::XMLElement ctor.
    //@}

    /** \name Accessors */ //@{
    GG::XMLElement XMLEncode();
    //@}

    int         id;
    std::string name;
    std::string player_name;
    GG::Clr     color;
};

/** The data structure used to represent a single player's setup options for a multiplayer game (in the multiplayer lobby screen). */
struct PlayerSetupData
{
    /** \name Structors */ //@{
    PlayerSetupData(); ///< default ctor.
    PlayerSetupData(const GG::XMLElement& elem); ///< GG::XMLElement ctor.
    //@}

    /** \name Accessors */ //@{
    GG::XMLElement XMLEncode() const;
    //@}

    std::string empire_name;  ///< the name of the player's empire
    GG::Clr empire_color;     ///< the color used to represent this player's empire.
    int save_game_empire_id;  ///< when an MP save game is being loaded, this is the id of the empire that this player will play
};

#endif // _MultiplayerCommon_h_
