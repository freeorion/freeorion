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

/** Returns a vector of the names of all settings files that must be the same between the server and clients. */
const std::vector<std::string>& VersionSensitiveSettingsFiles();

/** Returns a map of all source files to their CVS revision numbers.  This is used to verify that the
    server and clients are all using the same versions of code. */
const std::map<std::string, std::string>& SourceFiles();

/** Saves the filename and revision number contained within \a file_id_string for later retrieval via SourceFiles().
    This function returns a dummy boolean value, allowing it to be executed at static initialization time, via a hack --
    simply declare a file-scope bool variable, and initialize it: bool temp_bool = RecordSourceFile(...).  If the same
    filename is registered more than once, an exception will be thrown.  \see RecordHeaderFile() has more info on how it
    and this function should be used. */
bool RecordSourceFile(const std::string& file_id_string);

/** Virtually identical to RecordSourceFile().  Header files should declare an inline function that returns the
    SVN-expanded file id string for that header, and the header file's matching source file should record that info
    using this function.  Example:<br>In MultiplayerCommon.h:
    \verbatim
    inline std::string MultiplayerCommonRevision()
    {return std::string("$Id$");}
    \endverbatim<br>In MultiplayerCommon.cpp:
    \verbatim
    namespace {
    ...
    bool temp_header_bool = RecordHeaderFile(MultiplayerCommonRevision());
    bool temp_source_bool = RecordSourceFile("$Id$");
    }
    \endverbatim
*/
bool RecordHeaderFile(const std::string& file_id_string);

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

inline std::string MultiplayerCommonRevision()
{return "$Id$";}

#endif // _MultiplayerCommon_h_
