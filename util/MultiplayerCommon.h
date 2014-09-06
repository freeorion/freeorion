// -*- C++ -*-
#ifndef _MultiplayerCommon_h_
#define _MultiplayerCommon_h_

#include "../universe/Enums.h"
#include "../network/Networking.h"
#include "Export.h"

#include <GG/Clr.h>

#include <list>
#include <set>
#include <vector>
#include <boost/serialization/access.hpp>



class XMLElement;

FO_COMMON_API extern const std::string MP_SAVE_FILE_EXTENSION;
FO_COMMON_API extern const std::string SP_SAVE_FILE_EXTENSION;

/** Returns an XML representation of a GG::Clr object. */
XMLElement ClrToXML(const GG::Clr& clr);

/** Returns a GG::Clr object constructed from its XML representation. */
FO_COMMON_API GG::Clr XMLToClr(const XMLElement& clr);

/** The data that represent the galaxy setup for a new game. */
struct FO_COMMON_API GalaxySetupData {
    /** \name Structors */ //@{
    GalaxySetupData() :
        m_seed(),
        m_size(100),
        m_shape(SPIRAL_2),
        m_age(GALAXY_SETUP_MEDIUM),
        m_starlane_freq(GALAXY_SETUP_MEDIUM),
        m_planet_density(GALAXY_SETUP_MEDIUM),
        m_specials_freq(GALAXY_SETUP_MEDIUM),
        m_monster_freq(GALAXY_SETUP_MEDIUM),
        m_native_freq(GALAXY_SETUP_MEDIUM),
        m_ai_aggr(MANIACAL)
    {}
    //@}

    std::string         m_seed;
    int                 m_size;
    Shape               m_shape;
    GalaxySetupOption   m_age;
    GalaxySetupOption   m_starlane_freq;
    GalaxySetupOption   m_planet_density;
    GalaxySetupOption   m_specials_freq;
    GalaxySetupOption   m_monster_freq;
    GalaxySetupOption   m_native_freq;
    Aggression          m_ai_aggr;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Contains the UI data that must be saved in save game files in order to
  * restore games to the users' last views. */
struct FO_COMMON_API SaveGameUIData {
    int     map_top;
    int     map_left;
    double  map_zoom_steps_in;
    std::set<int> fleets_exploring;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The data for one empire necessary for game-setup during multiplayer loading. */
struct FO_COMMON_API SaveGameEmpireData {
    /** \name Structors */ //@{
    SaveGameEmpireData() :
        m_empire_id(ALL_EMPIRES),
        m_empire_name(),
        m_player_name(),
        m_color()
    {}
    SaveGameEmpireData(int empire_id, const std::string& empire_name,
                       const std::string& player_name, const GG::Clr& colour) :
        m_empire_id(empire_id),
        m_empire_name(empire_name),
        m_player_name(player_name),
        m_color(colour)
    {}
    //@}

    int         m_empire_id;
    std::string m_empire_name;
    std::string m_player_name;
    GG::Clr     m_color;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The data structure used to represent a single player's setup options for a
  * multiplayer game (in the multiplayer lobby screen). */
struct PlayerSetupData {
    /** \name Structors */ //@{
    PlayerSetupData() :
        m_player_name(),
        m_empire_name(),
        m_empire_color(GG::Clr(0, 0, 0, 0)),
        m_starting_species_name(),
        m_save_game_empire_id(ALL_EMPIRES),
        m_client_type(Networking::INVALID_CLIENT_TYPE)
    {}
    //@}

    std::string             m_player_name;          ///< the player's name

    std::string             m_empire_name;          ///< the name of the player's empire when starting a new game
    GG::Clr                 m_empire_color;         ///< the color used to represent this player's empire when starting a new game
    std::string             m_starting_species_name;///< name of the species with which the player starts when starting a new game

    int                     m_save_game_empire_id;  ///< when loading a game, the ID of the empire that this player will control

    Networking::ClientType  m_client_type;          ///< is this player an AI, human player or...?

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};
bool FO_COMMON_API operator==(const PlayerSetupData& lhs, const PlayerSetupData& rhs);
bool operator!=(const PlayerSetupData& lhs, const PlayerSetupData& rhs);


/** The data needed to establish a new single player game.  If \a m_new_game
  * is true, a new game is to be started, using the remaining members besides
  * \a m_filename.  Otherwise, the saved game \a m_filename will be loaded
  * instead. */
struct SinglePlayerSetupData : public GalaxySetupData {
    /** \name Structors */ //@{
    SinglePlayerSetupData():
        m_new_game(true),
        m_filename(),
        m_players()
    {}
    //@}

    bool                                m_new_game;
    std::string                         m_filename;
    std::vector<PlayerSetupData>        m_players;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The data structure that represents the state of the multiplayer lobby. */
struct FO_COMMON_API MultiplayerLobbyData : public GalaxySetupData {
    /** \name Structors */ //@{
    MultiplayerLobbyData() :
        m_new_game(true),
        m_players(),
        m_save_game(),
        m_save_game_empire_data()
    {}
    //@}

    std::string Dump() const;

    bool                                        m_new_game;
    std::list<std::pair<int, PlayerSetupData> > m_players;              // <player_id, PlayerSetupData>

    std::string                                 m_save_game;            //< File name of a save file
    std::map<int, SaveGameEmpireData>           m_save_game_empire_data;// indexed by empire_id

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Information about one player that other players are informed of.  Assembled by server and sent to players. */
struct PlayerInfo {
    PlayerInfo() :
        name(""),
        empire_id(ALL_EMPIRES),
        client_type(Networking::INVALID_CLIENT_TYPE),
        host(false)
    {}
    PlayerInfo(const std::string& player_name_, int empire_id_,
               Networking::ClientType client_type_, bool host_) :
        name(player_name_),
        empire_id(empire_id_),
        client_type(client_type_),
        host(host_)
    {}

    std::string             name;           ///< name of this player (not the same as the empire name)
    int                     empire_id;      ///< id of the player's empire
    Networking::ClientType  client_type;    ///< is this a human player, AI player, or observer?
    bool                    host;           ///< true iff this is the host player

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// Note: *::serialize() implemented in SerializeMultiplayerCommon.cpp.

#endif // _MultiplayerCommon_h_
