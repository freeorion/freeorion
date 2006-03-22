// -*- C++ -*-
#ifndef _MultiplayerCommon_h_
#define _MultiplayerCommon_h_

#include "XMLDoc.h"

#include <GG/Clr.h>

#include <vector>

/** The colors that are available for use for empires in the game. */
const std::vector<GG::Clr>& EmpireColors();

/** Returns an XML representation of a GG::Clr object. */
XMLElement ClrToXML(const GG::Clr& clr);

/** Returns a GG::Clr object constructed from its XML representation. */
GG::Clr XMLToClr(const XMLElement& clr);

/** Returns the integer priority level that should be passed to log4cpp for a given priority name string. */
int PriorityValue(const std::string& name);

/** Returns an MD5 "sum" of the given string as a 32-digithexidecimal string. */
std::string MD5StringSum(const std::string& str);

/** Returns an MD5 "sum" of the given text file as a 32-digit hexidecimal string. */
std::string MD5FileSum(const std::string& filename);

/** Returns a language-specific string for the key-string \a str */
const std::string& UserString(const std::string& str);

/** Returns the language of the StringTable currently in use */
const std::string& Language();

/** The data for one empire necessary for game-setup during multiplayer loading. */
struct SaveGameEmpireData
{
    /** \name Structors */ //@{
    SaveGameEmpireData(); ///< default ctor.
    SaveGameEmpireData(const XMLElement& elem); ///< XMLElement ctor.
    //@}

    /** \name Accessors */ //@{
    XMLElement XMLEncode();
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
    PlayerSetupData(const XMLElement& elem); ///< XMLElement ctor.
    //@}

    /** \name Accessors */ //@{
    XMLElement XMLEncode() const;
    //@}

    std::string empire_name;  ///< the name of the player's empire
    GG::Clr empire_color;     ///< the color used to represent this player's empire.
    int save_game_empire_id;  ///< when an MP save game is being loaded, this is the id of the empire that this player will play
};

#endif // _MultiplayerCommon_h_
