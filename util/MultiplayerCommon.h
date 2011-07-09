// -*- C++ -*-
#ifndef _MultiplayerCommon_h_
#define _MultiplayerCommon_h_

#include "Serialize.h"
#include "XMLDoc.h"
#include "../combat/OpenSteer/PathingEngine.h"
#include "../universe/Enums.h"
#include "../network/Networking.h"

#include <GG/Clr.h>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/format.hpp>

#include <set>
#include <vector>


class System;

extern const std::string MP_SAVE_FILE_EXTENSION;
extern const std::string SP_SAVE_FILE_EXTENSION;

/** The colors that are available for use for empires in the game. */
const std::vector<GG::Clr>& EmpireColors();

/** Returns an XML representation of a GG::Clr object. */
XMLElement ClrToXML(const GG::Clr& clr);

/** Returns a GG::Clr object constructed from its XML representation. */
GG::Clr XMLToClr(const XMLElement& clr);

/** Returns the integer priority level that should be passed to log4cpp for a given priority name string. */
int PriorityValue(const std::string& name);

/** Returns a language-specific string for the key-string \a str */
const std::string& UserString(const std::string& str);

/** Wraps boost::format such that it won't crash if passed the wrong number of arguments */
boost::format FlexibleFormat(const std::string &string_to_format);

/** Returns the stringified form of \a n as a roman number.  "Only" defined for 1 <= n <= 3999, as we can't display the
    symbol for 5000. */
std::string RomanNumber(unsigned int n);

/** Returns the language of the StringTable currently in use */
const std::string& Language();

#ifndef FREEORION_WIN32
/** Puts the calling thread to sleep for \a ms milliseconds. */
void Sleep(int ms);
#endif

/** The data that represent the galaxy setup for a new game. */
struct GalaxySetupData
{
    /** \name Structors */ //@{
    GalaxySetupData(); ///< default ctor.
    //@}

    int               m_size;
    Shape             m_shape;
    Age               m_age;
    StarlaneFrequency m_starlane_freq;
    PlanetDensity     m_planet_density;
    SpecialsFrequency m_specials_freq;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Contains the UI data that must be saved in save game files in order to
  * restore games to the users' last views. */
struct SaveGameUIData
{
    int     map_top;
    int     map_left;
    double  map_zoom_steps_in;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The data for one empire necessary for game-setup during multiplayer loading. */
struct SaveGameEmpireData
{
    /** \name Structors */ //@{
    SaveGameEmpireData(); ///< default ctor.
    SaveGameEmpireData(int empire_id, const std::string& empire_name, const std::string& player_name, const GG::Clr& colour);
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
struct PlayerSetupData
{
    /** \name Structors */ //@{
    PlayerSetupData(); ///< default ctor.
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

/** The data needed to establish a new single player game.  If \a m_new_game
  * is true, a new game is to be started, using the remaining members besides
  * \a m_filename.  Otherwise, the saved game \a m_filename will be loaded
  * instead. */
struct SinglePlayerSetupData : public GalaxySetupData
{
    /** \name Structors */ //@{
    SinglePlayerSetupData(); ///< default ctor.
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
struct MultiplayerLobbyData : public GalaxySetupData
{
    /** \name Structors */ //@{
    MultiplayerLobbyData(); ///< Default ctor.
    explicit MultiplayerLobbyData(bool build_save_game_list); ///< Basic ctor.
    //@}

    bool                                        m_new_game;
    int                                         m_save_file_index;
    std::list<std::pair<int, PlayerSetupData> > m_players;              // <player_id, PlayerSetupData>

    std::vector<std::string>                    m_save_games;
    std::map<int, SaveGameEmpireData>           m_save_game_empire_data;// indexed by empire_id

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Information about one player that other players are informed of.  Assembled by server and sent to players. */
struct PlayerInfo
{
    PlayerInfo();   ///< default ctor
    PlayerInfo(const std::string& player_name_, int empire_id_, Networking::ClientType client_type_, bool host_);

    std::string             name;           ///< name of this player (not the same as the empire name)
    int                     empire_id;      ///< id of the player's empire
    Networking::ClientType  client_type;    ///< is this a human player, AI player, or observer?
    bool                    host;           ///< true iff this is the host player

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

struct CombatSetupGroup;

/** The state of combat (units, planets, their health, etc.) at the start of a
    round of combat. */
struct CombatData
{
    CombatData();
    CombatData(System* system, std::map<int, std::vector<CombatSetupGroup> >& setup_groups);

    unsigned int m_combat_turn_number;
    System* m_system;
    std::map<int, UniverseObject*> m_combat_universe;
    PathingEngine m_pathing_engine;

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const;
    template<class Archive>
    void load(Archive & ar, const unsigned int version);
    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

/** Regions in which the user is allowed or disallowed to place ships during
    combat setup. */
struct CombatSetupRegion
{
    /** The types of setup-regions. */
    enum Type {
        RING,           ///< A ring concentric with the System.
        ELLIPSE,        ///< An ellipse.
        PARTIAL_ELLIPSE ///< An angular portion of an ellipse.
    };

    CombatSetupRegion();
    CombatSetupRegion(float radius_begin, float radius_end);
    CombatSetupRegion(float centroid_x, float centroid_y, float radius);
    CombatSetupRegion(float centroid_x, float centroid_y, float radial_axis, float tangent_axis);
    CombatSetupRegion(float centroid_x, float centroid_y, float radial_axis, float tangent_axis,
                      float theta_begin, float theta_end);

    Type m_type;          ///< The type/shape of the region.
    float m_radius_begin; ///< The start radius of a ring.
    float m_radius_end;   ///< The end radius of a ring.
    float m_centroid[2];  ///< The (x, y) position of the ellipse centroid.
    float m_radial_axis;  ///< The length of the radial axis; the radial axis always points to the star.
    float m_tangent_axis; ///< The length of the tangent axis (perpendicular to the radial axis).
    float m_theta_begin;  ///< The start of the angular region of the ellipse.
    float m_theta_end;    ///< The end of the angular region of the ellipse.

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Returns true iff \a point falls within \a region. */
bool PointInRegion(double point[2], const CombatSetupRegion& region);

/** A group of ships and a description of where they may be placed. */
struct CombatSetupGroup
{
    CombatSetupGroup();

    std::set<int> m_ships;                    ///< The ships in this group.
    std::vector<CombatSetupRegion> m_regions; ///< The regions the ships are/are not allowed in.
    bool m_allow;                             ///< Whether the regions are allow-regions or deny-regions.

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// Note: *::serialize() implemented in SerializeMultiplayerCommon.cpp.

#endif // _MultiplayerCommon_h_
