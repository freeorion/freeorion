// -*- C++ -*-
#ifndef _MultiplayerCommon_h_
#define _MultiplayerCommon_h_

#include "XMLDoc.h"
#include "../universe/Enums.h"

#include <GG/Clr.h>

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <vector>

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

/** Returns the language of the StringTable currently in use */
const std::string& Language();

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

/** The data needed to establish a new single player game. */
struct SinglePlayerSetupData : public GalaxySetupData
{
    /** \name Structors */ //@{
    SinglePlayerSetupData(); ///< default ctor.
    //@}

    std::string       m_host_player_name;
    std::string       m_empire_name;
    GG::Clr           m_empire_color;
    int               m_AIs;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Contains the UI data that must be saved in save game files in order to restore games to the users' last views. */
struct SaveGameUIData
{
    struct NebulaData
    {
        std::string filename;
        GG::Pt      center;

    private:
        friend class boost::serialization::access;
        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };

    GG::Pt                  map_upper_left;
    double                  map_zoom_factor;
    std::vector<NebulaData> map_nebulae;

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
    //@}

    int         m_id;
    std::string m_name;
    std::string m_player_name;
    GG::Clr     m_color;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The data structure used to represent a single player's setup options for a multiplayer game (in the multiplayer lobby screen). */
struct PlayerSetupData
{
    /** \name Structors */ //@{
    PlayerSetupData(); ///< default ctor.
    //@}

    int         m_player_id;           ///< the player's id, assigned by the server
    std::string m_player_name;         ///< the player's name
    std::string m_empire_name;         ///< the name of the player's empire
    GG::Clr     m_empire_color;        ///< the color used to represent this player's empire.
    int         m_save_game_empire_id; ///< when an MP save game is being loaded, this is the id of the empire that this player will play

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

    /** \name Mutators */ //@{
    void RebuildSaveGameEmpireData(); ///< Rebuilds m_save_game_empire_data by reading player/Empire data from the current save file.
    //@}

    bool                            m_new_game;
    int                             m_save_file_index;
    std::vector<PlayerSetupData>    m_players;
    std::vector<PlayerSetupData>    m_AIs;

    std::vector<std::string>        m_save_games;
    std::vector<GG::Clr>            m_empire_colors;
    std::vector<SaveGameEmpireData> m_save_game_empire_data;

    static const std::string MP_SAVE_FILE_EXTENSION;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// template implementations
template <class Archive>
void GalaxySetupData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_shape)
        & BOOST_SERIALIZATION_NVP(m_age)
        & BOOST_SERIALIZATION_NVP(m_starlane_freq)
        & BOOST_SERIALIZATION_NVP(m_planet_density)
        & BOOST_SERIALIZATION_NVP(m_specials_freq);
}

template <class Archive>
void SinglePlayerSetupData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GalaxySetupData)
        & BOOST_SERIALIZATION_NVP(m_host_player_name)
        & BOOST_SERIALIZATION_NVP(m_empire_name)
        & BOOST_SERIALIZATION_NVP(m_empire_color)
        & BOOST_SERIALIZATION_NVP(m_AIs);
}

template <class Archive>
void SaveGameUIData::NebulaData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(filename)
        & BOOST_SERIALIZATION_NVP(center);
}

template <class Archive>
void SaveGameUIData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(map_upper_left)
        & BOOST_SERIALIZATION_NVP(map_zoom_factor)
        & BOOST_SERIALIZATION_NVP(map_nebulae);
}

template <class Archive>
void SaveGameEmpireData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_player_name)
        & BOOST_SERIALIZATION_NVP(m_color);
}

template <class Archive>
void PlayerSetupData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_player_id)
        & BOOST_SERIALIZATION_NVP(m_player_name)
        & BOOST_SERIALIZATION_NVP(m_empire_name)
        & BOOST_SERIALIZATION_NVP(m_empire_color)
        & BOOST_SERIALIZATION_NVP(m_save_game_empire_id);
}

template <class Archive>
void MultiplayerLobbyData::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GalaxySetupData)
        & BOOST_SERIALIZATION_NVP(m_new_game)
        & BOOST_SERIALIZATION_NVP(m_save_file_index)
        & BOOST_SERIALIZATION_NVP(m_players)
        // NOTE: We are not serializing the AIs on purpose; they are supposed to be server-side entities only.
        & BOOST_SERIALIZATION_NVP(m_save_games)
        & BOOST_SERIALIZATION_NVP(m_empire_colors)
        & BOOST_SERIALIZATION_NVP(m_save_game_empire_data);
}

#endif // _MultiplayerCommon_h_
