#ifndef _MultiplayerCommon_h_
#define _MultiplayerCommon_h_

#include "boost_fix.h"

#include "../universe/ConstantsFwd.h"
#include "../universe/EnumsFwd.h"
#include "../network/Networking.h"
#include "Enum.h"
#include "Export.h"
#include "OptionsDB.h"
#include "OrderSet.h"
#include "Pending.h"


#include <compare>
#include <list>
#include <set>
#include <vector>
#include <map>
#include <boost/date_time/posix_time/posix_time_types.hpp>


FO_COMMON_API extern const std::string MP_SAVE_FILE_EXTENSION;
FO_COMMON_API extern const std::string SP_SAVE_FILE_EXTENSION;

//! Types of universe shapes during galaxy generation
FO_ENUM(
    (Shape),
    ((INVALID_SHAPE, -1))
    ((SPIRAL_2))       ///< a two-armed spiral galaxy
    ((SPIRAL_3))       ///< a three-armed spiral galaxy
    ((SPIRAL_4))       ///< a four-armed spiral galaxy
    ((CLUSTER))        ///< a cluster galaxy
    ((ELLIPTICAL))     ///< an elliptical galaxy
    ((DISC))           ///< a disc shaped galaxy
    ((BOX))            ///< a rectangular shaped galaxy
    ((IRREGULAR))      ///< an irregular galaxy
    ((RING))           ///< a ring galaxy
    ((RANDOM))         ///< a random one of the other shapes
    ((GALAXY_SHAPES))   ///< the number of shapes in this enum (leave this last)
)

//! Returns a user readable string for a Shape
FO_COMMON_API const std::string& TextForGalaxyShape(Shape shape);


//! General-use option for galaxy setup picks with "more" or "less" options.
FO_ENUM(
    (GalaxySetupOptionGeneric),
    ((INVALID_GALAXY_SETUP_OPTION, -1))
    ((GALAXY_SETUP_NONE))
    ((GALAXY_SETUP_LOW))
    ((GALAXY_SETUP_MEDIUM))
    ((GALAXY_SETUP_HIGH))
    ((GALAXY_SETUP_RANDOM))
    ((NUM_GALAXY_SETUP_OPTIONS))
)
//! Returns a user readable string for a GalaxySetupOptionGeneric
FO_COMMON_API const std::string& TextForGalaxySetupSetting(GalaxySetupOptionGeneric gso);

//! Specific-use option for galaxy setup monster picks
FO_ENUM(
    (GalaxySetupOptionMonsterFreq),
    ((INVALID_MONSTER_SETUP_OPTION, -1))
    ((MONSTER_SETUP_NONE))
    ((MONSTER_SETUP_EXTREMELY_LOW))
    ((MONSTER_SETUP_VERY_LOW))
    ((MONSTER_SETUP_LOW))
    ((MONSTER_SETUP_MEDIUM))
    ((MONSTER_SETUP_HIGH))
    ((MONSTER_SETUP_VERY_HIGH))
    ((MONSTER_SETUP_EXTREMELY_HIGH))
    ((MONSTER_SETUP_RANDOM))
    ((NUM_GALAXY_SETUP_OPTION_MONSTER_FREQS))
)
//! Returns a user readable string for a GalaxySetupOptionGeneric
FO_COMMON_API const std::string& TextForGalaxySetupSetting(GalaxySetupOptionMonsterFreq gso);

//! Levels of AI Aggression during galaxy generation
FO_ENUM(
    (Aggression),
    ((INVALID_AGGRESSION, -1))
    ((BEGINNER))
    ((TURTLE))         ///< Very Defensive
    ((CAUTIOUS))       ///< Somewhat Defensive
    ((TYPICAL))        ///< Typical
    ((AGGRESSIVE))     ///< Aggressive
    ((MANIACAL))       ///< Very Aggressive
    ((NUM_AI_AGGRESSION_LEVELS))
)

//! Returns a user readable string for an Aggression
FO_COMMON_API const std::string& TextForAIAggression(Aggression a);

/** The data that represent the galaxy setup for a new game. */
struct FO_COMMON_API GalaxySetupData {
    GalaxySetupData() = default;
    GalaxySetupData(const GalaxySetupData&) = default;
    GalaxySetupData(GalaxySetupData&& base);

    const std::string&           GetSeed() const noexcept { return seed; }
    int                          GetSize() const noexcept { return size; }
    Shape                        GetShape() const;
    GalaxySetupOptionGeneric     GetAge() const;
    GalaxySetupOptionGeneric     GetStarlaneFreq() const;
    GalaxySetupOptionGeneric     GetPlanetDensity() const;
    GalaxySetupOptionGeneric     GetSpecialsFreq() const;
    GalaxySetupOptionMonsterFreq GetMonsterFreq() const;
    GalaxySetupOptionGeneric     GetNativeFreq() const;
    Aggression                   GetAggression() const noexcept { return ai_aggr; }
    const auto&                  GetGameRules() const noexcept { return game_rules; }
    const auto&                  GetGameUID() const noexcept { return game_uid; }

    void SetSeed(std::string new_seed);
    void SetGameUID(std::string game_uid);

    GalaxySetupData& operator=(const GalaxySetupData&) = default;

    std::string                  seed;
    int                          size = 100;
    Shape                        shape = Shape::SPIRAL_2;
    GalaxySetupOptionGeneric     age = GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    GalaxySetupOptionGeneric     starlane_freq = GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    GalaxySetupOptionGeneric     planet_density = GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    GalaxySetupOptionGeneric     specials_freq = GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    GalaxySetupOptionMonsterFreq monster_freq = GalaxySetupOptionMonsterFreq::MONSTER_SETUP_MEDIUM;
    GalaxySetupOptionGeneric     native_freq = GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM;
    Aggression                   ai_aggr = Aggression::MANIACAL;

    std::map<std::string, std::string>  game_rules;
    std::string                         game_uid;

    /** HACK! This must be set to the encoding empire's id when serializing a
      * GalaxySetupData, so that only the relevant parts of the galaxy data are
      * serialized.  The use of this local field is done just so I don't
      * have to rewrite any custom boost::serialization classes that implement
      * empire-dependent visibility. */
    int encoding_empire = ALL_EMPIRES; ///< used during serialization to globally set what empire knowledge to use
};

/** Contains the UI data that must be saved in save game files in order to
  * restore games to the users' last views. */
struct FO_COMMON_API SaveGameUIData {
    std::unordered_map<std::string, int> obsolete_ship_parts;
    std::vector<std::pair<int, boost::optional<std::pair<bool, int>>>> ordered_ship_design_ids_and_obsolete;
    std::vector<std::pair<std::string, std::pair<bool, int>>> ordered_ship_hull_and_obsolete;
    std::set<int> fleets_exploring;
    double map_zoom_steps_in = 0.0;
    int    map_top = 0;
    int    map_left = 0;
    int    obsolete_ui_event_count = 0;
};

/** The data for one empire necessary for game-setup during multiplayer loading. */
struct FO_COMMON_API SaveGameEmpireData {
    SaveGameEmpireData() = default;
    SaveGameEmpireData(int id, std::string ename, std::string pname,
                       std::array<uint8_t, 4> c, bool a, bool e, bool w) :
        empire_name(std::move(ename)),
        player_name(std::move(pname)),
        color(c),
        empire_id(id),
        authenticated(a),
        eliminated(e),
        won(w)
    {}

    std::string empire_name;
    std::string player_name;
    std::array<uint8_t, 4> color = {};
    int         empire_id = ALL_EMPIRES;
    bool        authenticated = false;
    bool        eliminated = false;
    bool        won = false;
};

/** Contains basic data about a player in a game. */
struct FO_COMMON_API PlayerSaveHeaderData {
    PlayerSaveHeaderData() = default;
    PlayerSaveHeaderData(std::string name_, int empire_id_, Networking::ClientType client_type_) :
        name(std::move(name_)),
        empire_id(empire_id_),
        client_type(client_type_)
    {}

    std::string             name;
    int                     empire_id = ALL_EMPIRES;
    Networking::ClientType  client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
};

/** Contains data that must be saved for a single player. */
struct FO_COMMON_API PlayerSaveGameData final : public PlayerSaveHeaderData {
    PlayerSaveGameData() = default;

    PlayerSaveGameData(std::string name, int empire_id, OrderSet orders_, SaveGameUIData ui_data_,
                       std::string save_state_string_, Networking::ClientType client_type);

    PlayerSaveGameData(std::string name, int empire_id, Networking::ClientType client_type);

    std::string    save_state_string;
    OrderSet       orders;
    SaveGameUIData ui_data;
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
    std::string             empire_name;
    std::string             starting_species_name;
    int                     player_id = Networking::INVALID_PLAYER_ID;
    int                     save_game_empire_id = ALL_EMPIRES; //! When loading a game, the ID of the empire that this player will control
    int                     starting_team = Networking::NO_TEAM_ID;
    std::array<uint8_t, 4>  empire_color = {};
    Networking::ClientType  client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
    bool                    player_ready = false;
    bool                    authenticated = false;
};

// ignores player_id
bool FO_COMMON_API operator==(const PlayerSetupData& lhs, const PlayerSetupData& rhs);

/** The data needed to establish a new single player game.  If \a m_new_game
  * is true, a new game is to be started, using the remaining members besides
  * \a m_filename.  Otherwise, the saved game \a m_filename will be loaded
  * instead. */
struct SinglePlayerSetupData final : public GalaxySetupData {
    SinglePlayerSetupData() = default;

    std::string                     filename;
    std::vector<PlayerSetupData>    players;
    bool                            new_game = true;
};

/** The data structure that represents the state of the multiplayer lobby. */
struct FO_COMMON_API MultiplayerLobbyData final : public GalaxySetupData {
    MultiplayerLobbyData() = default;

    MultiplayerLobbyData(const GalaxySetupData& base) :
        GalaxySetupData(base)
    {}

    MultiplayerLobbyData(GalaxySetupData&& base) :
        GalaxySetupData(std::move(base))
    {}

    [[nodiscard]] std::string Dump() const;

    std::string                                 start_lock_cause;
    std::string                                 save_game;            //< File name of a save file
    std::map<int, SaveGameEmpireData>           save_game_empire_data;// indexed by empire_id
    std::list<std::pair<int, PlayerSetupData>>  players;              // <player_id, PlayerSetupData>   // TODO: Change from a list<(player_id, PlayerSetupData)> where PlayerSetupData contain player_id to a vector of PlayerSetupData
    int                                         save_game_current_turn = 0;
    bool                                        in_game = false; ///< In-game lobby
    bool                                        any_can_edit = false;
    bool                                        new_game = true;
    bool                                        start_locked = false;
};

/** The data structure stores information about latest chat massages. */
struct FO_COMMON_API ChatHistoryEntity {
    std::string              player_name;
    std::string              text;
    boost::posix_time::ptime timestamp;
    std::array<uint8_t, 4>   text_color{{192, 192, 192, 255}};
};

/** Information about one player that other players are informed of.  Assembled by server and sent to players. */
struct PlayerInfo {
    std::string             name; //! Name of this player (not the same as the empire name)
    int                     empire_id = ALL_EMPIRES;
    Networking::ClientType  client_type = Networking::ClientType::INVALID_CLIENT_TYPE;
    bool                    host = false; //! true iff this is the host player

    static_assert(__cpp_impl_three_way_comparison);
#if !defined(__cpp_lib_three_way_comparison)
    [[nodiscard]] std::strong_ordering operator<=>(const PlayerInfo&) const = default;
#else
    [[nodiscard]] auto operator<=>(const PlayerInfo&) const = default;
#endif
};


#endif
