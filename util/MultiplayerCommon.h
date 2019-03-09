#ifndef _MultiplayerCommon_h_
#define _MultiplayerCommon_h_

#include "../universe/EnumsFwd.h"
#include "../network/Networking.h"
#include "Export.h"
#include "OptionsDB.h"
#include "Pending.h"
#include "Serialize.h"

#include <GG/Clr.h>

#include <list>
#include <set>
#include <vector>
#include <map>
#include <boost/serialization/access.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>


FO_COMMON_API extern const std::string MP_SAVE_FILE_EXTENSION;
FO_COMMON_API extern const std::string SP_SAVE_FILE_EXTENSION;
FO_COMMON_API extern const int ALL_EMPIRES;
FO_COMMON_API extern const int INVALID_GAME_TURN;


/** The data that represent the galaxy setup for a new game. */
struct FO_COMMON_API GalaxySetupData {
    /** \name Structors */ //@{
    GalaxySetupData();
    GalaxySetupData(const GalaxySetupData&) = default;
    GalaxySetupData(GalaxySetupData&& base);
    //@}

    /** \name Accessors */ //@{
    const std::string&  GetSeed() const;
    int                 GetSize() const;
    Shape               GetShape() const;
    GalaxySetupOption   GetAge() const;
    GalaxySetupOption   GetStarlaneFreq() const;
    GalaxySetupOption   GetPlanetDensity() const;
    GalaxySetupOption   GetSpecialsFreq() const;
    GalaxySetupOption   GetMonsterFreq() const;
    GalaxySetupOption   GetNativeFreq() const;
    Aggression          GetAggression() const;
    const std::vector<std::pair<std::string, std::string>>&
                        GetGameRules() const;
    const std::string&  GetGameUID() const;
    //@}

    /** \name Mutators */ //@{
    void                SetSeed(const std::string& seed);
    void                SetGameUID(const std::string& game_uid);
    //@}

    GalaxySetupData& operator=(const GalaxySetupData&) = default;

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
    std::vector<std::pair<std::string, std::string>>
                        m_game_rules;
    std::string         m_game_uid;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(GalaxySetupData, 2);


/** Contains the UI data that must be saved in save game files in order to
  * restore games to the users' last views. */
struct FO_COMMON_API SaveGameUIData {
    int     map_top;
    int     map_left;
    double  map_zoom_steps_in;
    std::set<int> fleets_exploring;

    // See DesignWnd.cpp for the usage of the following variables.
    int obsolete_ui_event_count;
    std::vector<std::pair<int, boost::optional<std::pair<bool, int>>>> ordered_ship_design_ids_and_obsolete;
    std::vector<std::pair<std::string, std::pair<bool, int>>> ordered_ship_hull_and_obsolete;
    std::unordered_map<std::string, int> obsolete_ship_parts;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
    template <class Archive>
    void legacy_serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(SaveGameUIData, 2);


/** The data for one empire necessary for game-setup during multiplayer loading. */
struct FO_COMMON_API SaveGameEmpireData {
    /** \name Structors */ //@{
    SaveGameEmpireData() :
        m_empire_id(ALL_EMPIRES),
        m_empire_name(),
        m_player_name(),
        m_color(),
        m_authenticated(false)
    {}
    SaveGameEmpireData(int empire_id, const std::string& empire_name,
                       const std::string& player_name, const GG::Clr& colour,
                       bool authenticated) :
        m_empire_id(empire_id),
        m_empire_name(empire_name),
        m_player_name(player_name),
        m_color(colour),
        m_authenticated(authenticated)
    {}
    //@}

    int         m_empire_id;
    std::string m_empire_name;
    std::string m_player_name;
    GG::Clr     m_color;
    bool        m_authenticated;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(SaveGameEmpireData, 1);

/** Contains basic data about a player in a game. */
struct FO_COMMON_API PlayerSaveHeaderData {
    PlayerSaveHeaderData() :
        m_name(),
        m_empire_id(ALL_EMPIRES),
        m_client_type(Networking::INVALID_CLIENT_TYPE)
    {}

    PlayerSaveHeaderData(const std::string& name, int empire_id,
                         Networking::ClientType client_type) :
        m_name(name),
        m_empire_id(empire_id),
        m_client_type(client_type)
    {}

    std::string             m_name;
    int                     m_empire_id;
    Networking::ClientType  m_client_type;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Contains data that must be saved for a single player. */
struct FO_COMMON_API PlayerSaveGameData : public PlayerSaveHeaderData {
    PlayerSaveGameData() :
        PlayerSaveHeaderData(),
        m_orders(),
        m_ui_data(),
        m_save_state_string(),
        m_ready(false)
    {}

    PlayerSaveGameData(const std::string& name, int empire_id,
                       const std::shared_ptr<OrderSet>& orders,
                       const std::shared_ptr<SaveGameUIData>& ui_data,
                       const std::string& save_state_string,
                       Networking::ClientType client_type,
                       bool ready) :
        PlayerSaveHeaderData(name, empire_id, client_type),
        m_orders(orders),
        m_ui_data(ui_data),
        m_save_state_string(save_state_string),
        m_ready(ready)
    {}

    std::shared_ptr<OrderSet>       m_orders;
    std::shared_ptr<SaveGameUIData> m_ui_data;
    std::string                     m_save_state_string;
    bool                            m_ready;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(PlayerSaveGameData, 1);

/** Data that must be retained by the server when saving and loading a
  * game that isn't player data or the universe */
struct FO_COMMON_API ServerSaveGameData {
    ServerSaveGameData() :
        m_current_turn(INVALID_GAME_TURN)
    {}

    ServerSaveGameData(int current_turn) :
        m_current_turn(current_turn)
    {}

    int m_current_turn;

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
        m_player_id(Networking::INVALID_PLAYER_ID),
        m_empire_name(),
        m_empire_color(GG::Clr(0, 0, 0, 0)),
        m_starting_species_name(),
        m_save_game_empire_id(ALL_EMPIRES),
        m_client_type(Networking::INVALID_CLIENT_TYPE),
        m_player_ready(false),
        m_authenticated(false)
    {}
    //@}

    std::string             m_player_name;          ///< the player's name
    int                     m_player_id;            ///< player id
    std::string             m_empire_name;          ///< the name of the player's empire when starting a new game
    GG::Clr                 m_empire_color;         ///< the color used to represent this player's empire when starting a new game
    std::string             m_starting_species_name;///< name of the species with which the player starts when starting a new game
    int                     m_save_game_empire_id;  ///< when loading a game, the ID of the empire that this player will control
    Networking::ClientType  m_client_type;          ///< is this player an AI, human player or...?
    bool                    m_player_ready;         ///< if player ready to play.
    bool                    m_authenticated;        ///< if player was authenticated

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};
bool FO_COMMON_API operator==(const PlayerSetupData& lhs, const PlayerSetupData& rhs);
bool operator!=(const PlayerSetupData& lhs, const PlayerSetupData& rhs);

BOOST_CLASS_VERSION(PlayerSetupData, 1);

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

    bool                            m_new_game;
    std::string                     m_filename;
    std::vector<PlayerSetupData>    m_players;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The data structure that represents the state of the multiplayer lobby. */
struct FO_COMMON_API MultiplayerLobbyData : public GalaxySetupData {
    /** \name Structors */ //@{
    MultiplayerLobbyData() :
        m_any_can_edit(false),
        m_new_game(true),
        m_start_locked(false),
        m_players(),
        m_save_game(),
        m_save_game_empire_data()
    {}

    MultiplayerLobbyData(GalaxySetupData&& base) :
        GalaxySetupData(std::move(base)),
        m_any_can_edit(false),
        m_new_game(true),
        m_start_locked(false),
        m_players(),
        m_save_game(),
        m_save_game_empire_data()
    {}
    //@}

    std::string Dump() const;

    bool                                        m_any_can_edit;
    bool                                        m_new_game;
    bool                                        m_start_locked;
    // TODO: Change from a list<(player_id, PlayerSetupData)> where
    // PlayerSetupData contain player_id to a vector of PlayerSetupData
    std::list<std::pair<int, PlayerSetupData>>  m_players;              // <player_id, PlayerSetupData>

    std::string                                 m_save_game;            //< File name of a save file
    std::map<int, SaveGameEmpireData>           m_save_game_empire_data;// indexed by empire_id

    std::string                                 m_start_lock_cause;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** The data structure stores information about latest chat massages. */
struct FO_COMMON_API ChatHistoryEntity {
    /** \name Structors */ //@{
    ChatHistoryEntity()
    {}
    //@}

    boost::posix_time::ptime    m_timestamp;
    std::string                 m_player_name;
    std::string                 m_text;
    GG::Clr                     m_text_color;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(ChatHistoryEntity, 1);

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
