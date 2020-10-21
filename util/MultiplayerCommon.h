#ifndef _MultiplayerCommon_h_
#define _MultiplayerCommon_h_

#include "../universe/EnumsFwd.h"
#include "../network/Networking.h"
#include "Export.h"
#include "OptionsDB.h"
#include "OrderSet.h"
#include "Pending.h"

#include <GG/Enum.h>

#include <list>
#include <set>
#include <vector>
#include <map>
#include <boost/date_time/posix_time/posix_time_types.hpp>


FO_COMMON_API extern const std::string MP_SAVE_FILE_EXTENSION;
FO_COMMON_API extern const std::string SP_SAVE_FILE_EXTENSION;
FO_COMMON_API extern const int ALL_EMPIRES;
FO_COMMON_API extern const int INVALID_GAME_TURN;


//! Types of universe shapes during galaxy generation
GG_ENUM(Shape,
    INVALID_SHAPE = -1,
    SPIRAL_2,       ///< a two-armed spiral galaxy
    SPIRAL_3,       ///< a three-armed spiral galaxy
    SPIRAL_4,       ///< a four-armed spiral galaxy
    CLUSTER,        ///< a cluster galaxy
    ELLIPTICAL,     ///< an elliptical galaxy
    DISC,           ///< a disc shaped galaxy
    BOX,            ///< a rectangular shaped galaxy
    IRREGULAR,      ///< an irregular galaxy
    RING,           ///< a ring galaxy
    RANDOM,         ///< a random one of the other shapes
    GALAXY_SHAPES   ///< the number of shapes in this enum (leave this last)
)

//! Returns a user readable string for a Shape
FO_COMMON_API const std::string& TextForGalaxyShape(Shape shape);


//! General-use option for galaxy setup picks with "more" or "less" options.
GG_ENUM(GalaxySetupOption,
    INVALID_GALAXY_SETUP_OPTION = -1,
    GALAXY_SETUP_NONE,
    GALAXY_SETUP_LOW,
    GALAXY_SETUP_MEDIUM,
    GALAXY_SETUP_HIGH,
    GALAXY_SETUP_RANDOM,
    NUM_GALAXY_SETUP_OPTIONS
)

//! Returns a user readable string for a GalaxySetupOption
FO_COMMON_API const std::string& TextForGalaxySetupSetting(GalaxySetupOption gso);


//! Levels of AI Aggression during galaxy generation
GG_ENUM(Aggression,
    INVALID_AGGRESSION = -1,
    BEGINNER,
    TURTLE,         ///< Very Defensive
    CAUTIOUS,       ///< Somewhat Defensive
    TYPICAL,        ///< Typical
    AGGRESSIVE,     ///< Aggressive
    MANIACAL,       ///< Very Aggressive
    NUM_AI_AGGRESSION_LEVELS
)

//! Returns a user readable string for an Aggression
FO_COMMON_API const std::string& TextForAIAggression(Aggression a);


/** The data that represent the galaxy setup for a new game. */
struct FO_COMMON_API GalaxySetupData {
    GalaxySetupData();
    GalaxySetupData(const GalaxySetupData&) = default;
    GalaxySetupData(GalaxySetupData&& base);

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
    const std::map<std::string, std::string>&
                        GetGameRules() const;
    const std::string&  GetGameUID() const;

    void                SetSeed(const std::string& seed);
    void                SetGameUID(const std::string& game_uid);

    GalaxySetupData& operator=(const GalaxySetupData&) = default;

    std::string         seed;
    int                 size;
    Shape               shape;
    GalaxySetupOption   age;
    GalaxySetupOption   starlane_freq;
    GalaxySetupOption   planet_density;
    GalaxySetupOption   specials_freq;
    GalaxySetupOption   monster_freq;
    GalaxySetupOption   native_freq;
    Aggression          ai_aggr;
    std::map<std::string, std::string>
                        game_rules;
    std::string         game_uid;

    /** HACK! This must be set to the encoding empire's id when serializing a
      * GalaxySetupData, so that only the relevant parts of the galaxy data are
      * serialized.  The use of this local field is done just so I don't
      * have to rewrite any custom boost::serialization classes that implement
      * empire-dependent visibility. */
    int                 encoding_empire; ///< used during serialization to globally set what empire knowledge to use
};


/** Contains the UI data that must be saved in save game files in order to
  * restore games to the users' last views. */
struct FO_COMMON_API SaveGameUIData {
    int     map_top = 0;
    int     map_left = 0;
    double  map_zoom_steps_in = 0.0;
    std::set<int> fleets_exploring;

    // See DesignWnd.cpp for the usage of the following variables.
    int obsolete_ui_event_count = 0;
    std::vector<std::pair<int, boost::optional<std::pair<bool, int>>>> ordered_ship_design_ids_and_obsolete;
    std::vector<std::pair<std::string, std::pair<bool, int>>> ordered_ship_hull_and_obsolete;
    std::unordered_map<std::string, int> obsolete_ship_parts;
};


/** The data for one empire necessary for game-setup during multiplayer loading. */
struct FO_COMMON_API SaveGameEmpireData {
    int         empire_id = ALL_EMPIRES;
    std::string empire_name;
    std::string player_name;
    std::array<unsigned char, 4> color;
    bool        authenticated = false;
    bool        eliminated = false;
    bool        won = false;
};

/** Contains basic data about a player in a game. */
struct FO_COMMON_API PlayerSaveHeaderData {
    std::string             name;
    int                     empire_id = ALL_EMPIRES;
    Networking::ClientType  client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
};

/** Contains data that must be saved for a single player. */
struct FO_COMMON_API PlayerSaveGameData : public PlayerSaveHeaderData {
    PlayerSaveGameData() :
        PlayerSaveHeaderData()
    {}

    PlayerSaveGameData(const std::string& name, int empire_id,
                       const std::shared_ptr<OrderSet>& orders_,
                       const std::shared_ptr<SaveGameUIData>& ui_data_,
                       const std::string& save_state_string_,
                       Networking::ClientType client_type) :
        PlayerSaveHeaderData{name, empire_id, client_type},
        orders(orders_),
        ui_data(ui_data_),
        save_state_string(save_state_string_)
    {}

    std::shared_ptr<OrderSet>       orders;
    std::shared_ptr<SaveGameUIData> ui_data;
    std::string                     save_state_string;
};

/** Data that must be retained by the server when saving and loading a
  * game that isn't player data or the universe */
struct FO_COMMON_API ServerSaveGameData {
    int current_turn = INVALID_GAME_TURN;
};

/** The data structure used to represent a single player's setup options for a
  * multiplayer game (in the multiplayer lobby screen). */
struct PlayerSetupData {
    std::string             player_name;
    int                     player_id = Networking::INVALID_PLAYER_ID;
    std::string             empire_name;
    std::array<unsigned char, 4> empire_color{0, 0, 0, 0};
    std::string             starting_species_name;
    //! When loading a game, the ID of the empire that this player will control
    int                     save_game_empire_id = ALL_EMPIRES;
    Networking::ClientType  client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
    bool                    player_ready = false;
    bool                    authenticated = false;
    int                     starting_team = Networking::NO_TEAM_ID;
};

bool FO_COMMON_API operator==(const PlayerSetupData& lhs, const PlayerSetupData& rhs);
bool operator!=(const PlayerSetupData& lhs, const PlayerSetupData& rhs);

/** The data needed to establish a new single player game.  If \a m_new_game
  * is true, a new game is to be started, using the remaining members besides
  * \a m_filename.  Otherwise, the saved game \a m_filename will be loaded
  * instead. */
struct SinglePlayerSetupData : public GalaxySetupData {
    SinglePlayerSetupData():
        new_game(true),
        filename(),
        players()
    {}

    bool                            new_game;
    std::string                     filename;
    std::vector<PlayerSetupData>    players;
};

/** The data structure that represents the state of the multiplayer lobby. */
struct FO_COMMON_API MultiplayerLobbyData : public GalaxySetupData {
    MultiplayerLobbyData() :
        any_can_edit(false),
        new_game(true),
        start_locked(false),
        players(),
        save_game(),
        save_game_empire_data(),
        save_game_current_turn(0),
        in_game(false)
    {}

    MultiplayerLobbyData(const GalaxySetupData& base) :
        GalaxySetupData(base),
        any_can_edit(false),
        new_game(true),
        start_locked(false),
        players(),
        save_game(),
        save_game_empire_data(),
        save_game_current_turn(0),
        in_game(false)
    {}

    MultiplayerLobbyData(GalaxySetupData&& base) :
        GalaxySetupData(std::move(base)),
        any_can_edit(false),
        new_game(true),
        start_locked(false),
        players(),
        save_game(),
        save_game_empire_data(),
        save_game_current_turn(0),
        in_game(false)
    {}

    std::string Dump() const;

    bool                                        any_can_edit;
    bool                                        new_game;
    bool                                        start_locked;
    // TODO: Change from a list<(player_id, PlayerSetupData)> where
    // PlayerSetupData contain player_id to a vector of PlayerSetupData
    std::list<std::pair<int, PlayerSetupData>>  players;              // <player_id, PlayerSetupData>

    std::string                                 save_game;            //< File name of a save file
    std::map<int, SaveGameEmpireData>           save_game_empire_data;// indexed by empire_id
    int                                         save_game_current_turn;

    std::string                                 start_lock_cause;
    bool                                        in_game; ///< In-game lobby
};

/** The data structure stores information about latest chat massages. */
struct FO_COMMON_API ChatHistoryEntity {
    boost::posix_time::ptime    timestamp;
    std::string                 player_name;
    std::string                 text;
    std::array<unsigned char, 4> text_color;
};

/** Information about one player that other players are informed of.  Assembled by server and sent to players. */
struct PlayerInfo {
    //! Name of this player (not the same as the empire name)
    std::string             name;
    int                     empire_id = ALL_EMPIRES;
    Networking::ClientType  client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
    //! true iff this is the host player
    bool                    host = false;
};


#endif
