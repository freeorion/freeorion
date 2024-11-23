#include "MultiplayerCommon.h"

#include "Directories.h"
#include "GameRules.h"
#include "GameRuleRanks.h"
#include "i18n.h"
#include "LoggerWithOptionsDB.h"
#include "OptionsDB.h"
#include "Random.h"
#include "AppInterface.h"


#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

const std::string MP_SAVE_FILE_EXTENSION = ".mps";
const std::string SP_SAVE_FILE_EXTENSION = ".sav";


namespace {
    // command-line options
    void AddOptions(OptionsDB& db) {
#ifdef FREEORION_ANDROID
        db.Add<std::string>("resource.path",                UserStringNop("OPTIONS_DB_RESOURCE_DIR"),           "default");
#else
        db.Add<std::string>("resource.path",                UserStringNop("OPTIONS_DB_RESOURCE_DIR"),           PathToString(GetRootDataDir() / "default"));
#endif
        db.Add<std::string>('S', "save.path",               UserStringNop("OPTIONS_DB_SAVE_DIR"),               PathToString(GetUserDataDir() / "save"));
        db.Add<std::string>("save.server.path",             UserStringNop("OPTIONS_DB_SERVER_SAVE_DIR"),        PathToString(GetUserDataDir() / "save"));
        db.Add<std::string>("log-level",                    UserStringNop("OPTIONS_DB_LOG_LEVEL"),              "",
                            OrValidator<std::string>(LogLevelValidator(), std::make_unique<DiscreteValidator<std::string>>("")),
                            false);
        db.Add<std::string>("log-file",                     UserStringNop("OPTIONS_DB_LOG_FILE"),               "",
                            Validator<std::string>(),                                                           false);
        // Default stringtable filename is deferred to i18n.cpp::InitStringtableFileName
        db.Add<std::string>("resource.stringtable.path",    UserStringNop("OPTIONS_DB_STRINGTABLE_FILENAME"),   "");
        db.Add("save.format.binary.enabled",                UserStringNop("OPTIONS_DB_BINARY_SERIALIZATION"),   false);
        db.Add("save.format.xml.zlib.enabled",              UserStringNop("OPTIONS_DB_XML_ZLIB_SERIALIZATION"), true);
        db.Add("save.auto.hostless.enabled",                UserStringNop("OPTIONS_DB_AUTOSAVE_HOSTLESS"),      true);
        db.Add("save.auto.hostless.each-player.enabled",    UserStringNop("OPTIONS_DB_AUTOSAVE_HOSTLESS_EACH_PLAYER"), false);
        db.Add<int>("save.auto.interval",                   UserStringNop("OPTIONS_DB_AUTOSAVE_INTERVAL"),      0);
        db.Add<std::string>("load",                         UserStringNop("OPTIONS_DB_LOAD"),                   "",                           Validator<std::string>(), false);
        db.Add("save.auto.exit.enabled",                    UserStringNop("OPTIONS_DB_AUTOSAVE_GAME_CLOSE"),    true);
        db.AddFlag('q', "quickstart",                       UserStringNop("OPTIONS_DB_QUICKSTART"),             false);

        // Common galaxy settings
        db.Add("setup.seed",                UserStringNop("OPTIONS_DB_GAMESETUP_SEED"),               std::string("0"),                       Validator<std::string>());
        db.Add("setup.star.count",          UserStringNop("OPTIONS_DB_GAMESETUP_STARS"),              150,                                    RangedValidator<int>(10, 5000));
        db.Add("setup.galaxy.shape",        UserStringNop("OPTIONS_DB_GAMESETUP_GALAXY_SHAPE"),       Shape::DISC,                            RangedValidator<Shape>(Shape::SPIRAL_2, Shape::RANDOM));
        db.Add("setup.galaxy.age",          UserStringNop("OPTIONS_DB_GAMESETUP_GALAXY_AGE"),         GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM, RangedValidator<GalaxySetupOptionGeneric>(GalaxySetupOptionGeneric::GALAXY_SETUP_LOW, GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM));
        db.Add("setup.planet.density",      UserStringNop("OPTIONS_DB_GAMESETUP_PLANET_DENSITY"),     GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM, RangedValidator<GalaxySetupOptionGeneric>(GalaxySetupOptionGeneric::GALAXY_SETUP_LOW, GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM));
        db.Add("setup.starlane.frequency",  UserStringNop("OPTIONS_DB_GAMESETUP_STARLANE_FREQUENCY"), GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM, RangedValidator<GalaxySetupOptionGeneric>(GalaxySetupOptionGeneric::GALAXY_SETUP_LOW, GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM));
        db.Add("setup.specials.frequency",  UserStringNop("OPTIONS_DB_GAMESETUP_SPECIALS_FREQUENCY"), GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM, RangedValidator<GalaxySetupOptionGeneric>(GalaxySetupOptionGeneric::GALAXY_SETUP_NONE, GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM));
        db.Add("setup.monster.frequency",   UserStringNop("OPTIONS_DB_GAMESETUP_MONSTER_FREQUENCY"),  GalaxySetupOptionMonsterFreq::MONSTER_SETUP_MEDIUM, RangedValidator<GalaxySetupOptionMonsterFreq>(GalaxySetupOptionMonsterFreq::MONSTER_SETUP_NONE, GalaxySetupOptionMonsterFreq::MONSTER_SETUP_RANDOM));
        db.Add("setup.native.frequency",    UserStringNop("OPTIONS_DB_GAMESETUP_NATIVE_FREQUENCY"),   GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM, RangedValidator<GalaxySetupOptionGeneric>(GalaxySetupOptionGeneric::GALAXY_SETUP_NONE, GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM));
        db.Add("setup.ai.player.count",     UserStringNop("OPTIONS_DB_GAMESETUP_NUM_AI_PLAYERS"),     6,                                      RangedValidator<int>(0, IApp::MAX_AI_PLAYERS()));
        db.Add("setup.ai.aggression",       UserStringNop("OPTIONS_DB_GAMESETUP_AI_MAX_AGGRESSION"),  Aggression::MANIACAL,                   RangedValidator<Aggression>(Aggression::BEGINNER, Aggression::MANIACAL));


        // AI Testing options-- the following options are to facilitate AI testing and do not currently have an options page widget;
        // they are intended to be changed via the command line and are not currently storable in the configuration file.
        db.Add<std::string>("ai-path",      UserStringNop("OPTIONS_DB_AI_FOLDER_PATH"),               "python/AI",                            nullptr, false);
        db.Add<std::string>("ai-config",    UserStringNop("OPTIONS_DB_AI_CONFIG"),                    "",                                     nullptr, false);
        db.Add<std::string>("ai-log-dir",   UserStringNop("OPTIONS_DB_AI_LOG_DIR"),                   "",                                     nullptr, false);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    void AddRules(GameRules& rules) {
        rules.Add<int>(UserStringNop("RULE_THRESHOLD_HUMAN_PLAYER_WIN"),
                       UserStringNop("RULE_THRESHOLD_HUMAN_PLAYER_WIN_DESC"),
                       GameRuleCategories::GameRuleCategory::MULTIPLAYER,
                       0, true,
                       GameRuleRanks::RULE_THRESHOLD_HUMAN_PLAYER_WIN_RANK,
                       RangedValidator<int>(0, 999));

        rules.Add<bool>(UserStringNop("RULE_ONLY_ALLIANCE_WIN"),
                        UserStringNop("RULE_ONLY_ALLIANCE_WIN_DESC"),
                        GameRuleCategories::GameRuleCategory::MULTIPLAYER,
                        true, true,
                        GameRuleRanks::RULE_ONLY_ALLIANCE_WIN_RANK);

        rules.Add<bool>(UserStringNop("RULE_ALLOW_CONCEDE"),
                        UserStringNop("RULE_ALLOW_CONCEDE_DESC"),
                        GameRuleCategories::GameRuleCategory::MULTIPLAYER,
                        false, true,
                        GameRuleRanks::RULE_ALLOW_CONCEDE_RANK);

        rules.Add<int>(UserStringNop("RULE_CONCEDE_COLONIES_THRESHOLD"),
                       UserStringNop("RULE_CONCEDE_COLONIES_THRESHOLD_DESC"),
                       GameRuleCategories::GameRuleCategory::MULTIPLAYER,
                       1, true,
                       GameRuleRanks::RULE_CONCEDE_COLONIES_THRESHOLD_RANK,
                       RangedValidator<int>(0, 9999));

        rules.Add<bool>(UserStringNop("RULE_CONCEDE_DESTROY_COLONIES"),
                        UserStringNop("RULE_CONCEDE_DESTROY_COLONIES_DESC"),
                        GameRuleCategories::GameRuleCategory::MULTIPLAYER,
                        false, true,
                        GameRuleRanks::RULE_CONCEDE_DESTROY_COLONIES_RANK);

        rules.Add<bool>(UserStringNop("RULE_CONCEDE_DESTROY_BUILDINGS"),
                        UserStringNop("RULE_CONCEDE_DESTROY_BUILDINGS_DESC"),
                        GameRuleCategories::GameRuleCategory::MULTIPLAYER,
                        false, true,
                        GameRuleRanks::RULE_CONCEDE_DESTROY_BUILDINGS_RANK);

        rules.Add<bool>(UserStringNop("RULE_CONCEDE_DESTROY_SHIPS"),
                        UserStringNop("RULE_CONCEDE_DESTROY_SHIPS_DESC"),
                        GameRuleCategories::GameRuleCategory::MULTIPLAYER,
                        false, true,
                        GameRuleRanks::RULE_CONCEDE_DESTROY_SHIPS_RANK);
    }
    bool temp_bool2 = RegisterGameRules(&AddRules);

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif
}

/////////////////////////////////////////////////////
// GalaxySetupData
/////////////////////////////////////////////////////
namespace {
    // returns number in range 0 to one less than the interger representation of
    // enum_vals_count, determined by the random seed
    template <typename T1>
    int GetIdx(const T1& enum_vals_count, const std::string& seed) {
        DebugLogger() << "hashing seed: " << seed;
        // use probably-bad but adequate for this purpose hash function to
        // convert seed into a hash value
        int hash_value = 223;
        for (std::size_t i = 0; i < seed.length(); ++i) {
            //DebugLogger() << "hash value: " << hash_value << " char: " << static_cast<int>(seed[i]);
            hash_value += (seed[i] * 61);
            hash_value %= 191;
        }
        DebugLogger() << "final hash value: " << hash_value
                      << " and returning: " << hash_value % static_cast<int>(enum_vals_count)
                      << " from 0 to " << static_cast<int>(enum_vals_count) - 1;
        return hash_value % static_cast<int>(enum_vals_count);
    }

    constexpr std::string_view alphanum = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
}

const std::string& TextForGalaxySetupSetting(GalaxySetupOptionGeneric gso) {
    switch (gso) {
        case GalaxySetupOptionGeneric::GALAXY_SETUP_NONE:   return UserString("GSETUP_NONE");
        case GalaxySetupOptionGeneric::GALAXY_SETUP_LOW:    return UserString("GSETUP_LOW");
        case GalaxySetupOptionGeneric::GALAXY_SETUP_MEDIUM: return UserString("GSETUP_MEDIUM");
        case GalaxySetupOptionGeneric::GALAXY_SETUP_HIGH:   return UserString("GSETUP_HIGH");
        case GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM: return UserString("GSETUP_RANDOM");
        default:                                            return EMPTY_STRING;
    }
}

const std::string& TextForGalaxySetupSetting(GalaxySetupOptionMonsterFreq gso) {
    switch (gso) {
        case GalaxySetupOptionMonsterFreq::MONSTER_SETUP_NONE:           return UserString("GSETUP_NONE");
        case GalaxySetupOptionMonsterFreq::MONSTER_SETUP_EXTREMELY_LOW:  return UserString("GSETUP_EXTREMELY_LOW");
        case GalaxySetupOptionMonsterFreq::MONSTER_SETUP_VERY_LOW:       return UserString("GSETUP_VERY_LOW");
        case GalaxySetupOptionMonsterFreq::MONSTER_SETUP_LOW:            return UserString("GSETUP_LOW");
        case GalaxySetupOptionMonsterFreq::MONSTER_SETUP_MEDIUM:         return UserString("GSETUP_MEDIUM");
        case GalaxySetupOptionMonsterFreq::MONSTER_SETUP_HIGH:           return UserString("GSETUP_HIGH");
        case GalaxySetupOptionMonsterFreq::MONSTER_SETUP_VERY_HIGH:      return UserString("GSETUP_VERY_HIGH");
        case GalaxySetupOptionMonsterFreq::MONSTER_SETUP_EXTREMELY_HIGH: return UserString("GSETUP_EXTREMELY_HIGH");
        case GalaxySetupOptionMonsterFreq::MONSTER_SETUP_RANDOM:         return UserString("GSETUP_RANDOM");
        default:                                                         return EMPTY_STRING;
    }
}

const std::string& TextForGalaxyShape(Shape shape) {
    switch (shape) {
        case Shape::SPIRAL_2:   return UserString("GSETUP_2ARM");
        case Shape::SPIRAL_3:   return UserString("GSETUP_3ARM");
        case Shape::SPIRAL_4:   return UserString("GSETUP_4ARM");
        case Shape::CLUSTER:    return UserString("GSETUP_CLUSTER");
        case Shape::ELLIPTICAL: return UserString("GSETUP_ELLIPTICAL");
        case Shape::DISC:       return UserString("GSETUP_DISC");
        case Shape::BOX:        return UserString("GSETUP_BOX");
        case Shape::IRREGULAR:  return UserString("GSETUP_IRREGULAR");
        case Shape::RING:       return UserString("GSETUP_RING");
        case Shape::RANDOM:     return UserString("GSETUP_RANDOM");
        default:                return EMPTY_STRING;
    }
}

const std::string& TextForAIAggression(Aggression a) {
    switch (a) {
        case Aggression::BEGINNER:   return UserString("GSETUP_BEGINNER");
        case Aggression::TURTLE:     return UserString("GSETUP_TURTLE");
        case Aggression::CAUTIOUS:   return UserString("GSETUP_CAUTIOUS");
        case Aggression::TYPICAL:    return UserString("GSETUP_TYPICAL");
        case Aggression::AGGRESSIVE: return UserString("GSETUP_AGGRESSIVE");
        case Aggression::MANIACAL:   return UserString("GSETUP_MANIACAL");
        default:                     return EMPTY_STRING;
    }
}

GalaxySetupData::GalaxySetupData(GalaxySetupData&& base) :
    seed(std::move(base.seed)),
    size(base.size),
    shape(base.shape),
    age(base.age),
    starlane_freq(base.starlane_freq),
    planet_density(base.planet_density),
    specials_freq(base.specials_freq),
    monster_freq(base.monster_freq),
    native_freq(base.native_freq),
    ai_aggr(base.ai_aggr),
    game_rules(std::move(base.game_rules)),
    game_uid(std::move(base.game_uid)),
    encoding_empire(base.encoding_empire)
{ SetSeed(seed); }

Shape GalaxySetupData::GetShape() const {
    if (shape != Shape::RANDOM)
        return shape;
    std::size_t num_shapes = int(Shape::GALAXY_SHAPES) - 1; // -1 so that RANDOM isn't counted
    return static_cast<Shape>(GetIdx(num_shapes, seed + "shape"));
}

GalaxySetupOptionGeneric GalaxySetupData::GetAge() const {
    if (age != GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM)
        return age;
    return static_cast<GalaxySetupOptionGeneric>(GetIdx(3, seed + "age") + 1);       // need range 1-3 for age
}

GalaxySetupOptionGeneric GalaxySetupData::GetStarlaneFreq() const {
    if (starlane_freq != GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM)
        return starlane_freq;
    return static_cast<GalaxySetupOptionGeneric>(GetIdx(3, seed + "lanes") + 1);     // need range 1-3 for starlane freq
}

GalaxySetupOptionGeneric GalaxySetupData::GetPlanetDensity() const {
    if (planet_density != GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM)
        return planet_density;
    return static_cast<GalaxySetupOptionGeneric>(GetIdx(3, seed + "planets") + 1);   // need range 1-3 for planet density
}

GalaxySetupOptionGeneric GalaxySetupData::GetSpecialsFreq() const {
    if (specials_freq != GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM)
        return specials_freq;
    return static_cast<GalaxySetupOptionGeneric>(GetIdx(4, seed + "specials"));      // need range 0-3 for planet density
}

GalaxySetupOptionMonsterFreq GalaxySetupData::GetMonsterFreq() const {
    if (monster_freq != GalaxySetupOptionMonsterFreq::MONSTER_SETUP_RANDOM)
        return monster_freq;
    return static_cast<GalaxySetupOptionMonsterFreq>(GetIdx(8, seed + "monsters"));  // need range 0-7 for monster frequency
}

GalaxySetupOptionGeneric GalaxySetupData::GetNativeFreq() const {
    if (native_freq != GalaxySetupOptionGeneric::GALAXY_SETUP_RANDOM)
        return native_freq;
    return static_cast<GalaxySetupOptionGeneric>(GetIdx(4, seed + "natives"));       // need range 0-3 for native frequency
}

void GalaxySetupData::SetSeed(std::string new_seed) {
    if (new_seed.empty() || new_seed == "RANDOM") {
        ClockSeed();
        new_seed.clear();
        for (int i = 0; i < 8; ++i)
            new_seed += alphanum[RandInt(0, alphanum.size() - 2)];
        DebugLogger() << "Set empty or requested random seed to " << new_seed;
    }
    seed = std::move(new_seed);
}

void GalaxySetupData::SetGameUID(std::string game_uid_)
{ game_uid = std::move(game_uid_); }

/////////////////////////////////////////////////////
// PlayerSetupData
/////////////////////////////////////////////////////
bool operator==(const PlayerSetupData& lhs, const PlayerSetupData& rhs) {
    return  lhs.client_type == rhs.client_type &&
            lhs.empire_color == rhs.empire_color &&
            lhs.empire_name == rhs.empire_name &&
            lhs.player_name == rhs.player_name &&
            lhs.save_game_empire_id == rhs.save_game_empire_id &&
            lhs.starting_species_name == rhs.starting_species_name &&
            lhs.player_ready == rhs.player_ready &&
            lhs.starting_team == rhs.starting_team;
}

////////////////////////////////////////////////////
// MultiplayerLobbyData
/////////////////////////////////////////////////////
std::string MultiplayerLobbyData::Dump() const {
    std::stringstream stream;
    for (const std::pair<int, PlayerSetupData>& psd : players) {
        stream << psd.first << ": " << (psd.second.player_name.empty() ? "NO NAME" : psd.second.player_name) << "  ";
        if (psd.second.client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER)
            stream << "AI PLAYER";
        else if (psd.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER)
            stream << "HUMAN PLAYER";
        else if (psd.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER)
            stream << "HUMAN OBSERVER";
        else if (psd.second.client_type == Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR)
            stream << "HUMAN MODERATOR";
        else
            stream << "UNKNOWN CLIENT TPYE";
        stream << "  " << (psd.second.empire_name.empty() ? "NO EMPIRE NAME" : psd.second.empire_name) << "\n";
    }
    return stream.str();
}

////////////////////////////////////////////////////
// PlayerSaveGameData
/////////////////////////////////////////////////////
PlayerSaveGameData::PlayerSaveGameData(std::string name, int empire_id,
                                       OrderSet orders_, SaveGameUIData ui_data_,
                                       std::string save_state_string_,
                                       Networking::ClientType client_type) :
    PlayerSaveHeaderData(std::move(name), empire_id, client_type),
    save_state_string(std::move(save_state_string_)),
    orders(std::move(orders_)),
    ui_data(std::move(ui_data_))
{
    if (client_type != Networking::ClientType::CLIENT_TYPE_AI_PLAYER && save_state_string.empty())
        save_state_string = "NOT_SET_BY_CLIENT_TYPE";

    // The generation of the savegame data may be before any orders have been sent by clients. 
    // This is expected behaviour and to be handled differently by the AI than a possibly 
    // default-generated empty save_state_string.
    if (client_type == Networking::ClientType::CLIENT_TYPE_AI_PLAYER && orders.empty() && save_state_string.empty())
        save_state_string = "NO_STATE_YET";
}

PlayerSaveGameData::PlayerSaveGameData(std::string name, int empire_id, Networking::ClientType client_type) :
    PlayerSaveGameData(std::move(name), empire_id, OrderSet{}, SaveGameUIData{}, std::string{}, client_type)
{}