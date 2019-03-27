#include "MultiplayerCommon.h"

#include "Directories.h"
#include "i18n.h"
#include "LoggerWithOptionsDB.h"
#include "OptionsDB.h"
#include "Random.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Enums.h"

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif


const std::string MP_SAVE_FILE_EXTENSION = ".mps";
const std::string SP_SAVE_FILE_EXTENSION = ".sav";


namespace {
    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add<std::string>("resource.path",                UserStringNop("OPTIONS_DB_RESOURCE_DIR"),           PathToString(GetRootDataDir() / "default"));
        db.Add<std::string>('S', "save.path",               UserStringNop("OPTIONS_DB_SAVE_DIR"),               PathToString(GetUserDataDir() / "save"));
        db.Add<std::string>("save.server.path",             UserStringNop("OPTIONS_DB_SERVER_SAVE_DIR"),        PathToString(GetUserDataDir() / "save"));
        db.Add<std::string>("log-level",                    UserStringNop("OPTIONS_DB_LOG_LEVEL"),              "",
                            OrValidator<std::string>(LogLevelValidator(), DiscreteValidator<std::string>("")),  false);
        db.Add<std::string>("log-file",                     UserStringNop("OPTIONS_DB_LOG_FILE"),               "",
                            Validator<std::string>() ,                                                          false);
        // Default stringtable filename is deferred to i18n.cpp::InitStringtableFileName to determine if user specified
        db.Add<std::string>("resource.stringtable.path",    UserStringNop("OPTIONS_DB_STRINGTABLE_FILENAME"),   "");
        db.Add("save.format.binary.enabled",                UserStringNop("OPTIONS_DB_BINARY_SERIALIZATION"),   false);
        db.Add("save.format.xml.zlib.enabled",              UserStringNop("OPTIONS_DB_XML_ZLIB_SERIALIZATION"), true);
        db.Add("save.auto.hostless.enabled",                UserStringNop("OPTIONS_DB_AUTOSAVE_HOSTLESS"),      true);
        db.Add<int>("save.auto.interval",                   UserStringNop("OPTIONS_DB_AUTOSAVE_INTERVAL"),      0, Validator<int>());
        db.Add<std::string>("load",                         UserStringNop("OPTIONS_DB_LOAD"),                   "", Validator<std::string>(), false);

        // AI Testing options-- the following options are to facilitate AI testing and do not currently have an options page widget;
        // they are intended to be changed via the command line and are not currently storable in the configuration file.
        db.Add<std::string>("ai-path",                      UserStringNop("OPTIONS_DB_AI_FOLDER_PATH"),         "python/AI",
                            Validator<std::string>(),                                                           false);
        db.Add<std::string>("ai-config",                    UserStringNop("OPTIONS_DB_AI_CONFIG"),              "",
                            Validator<std::string>(),                                                           false);
    }
    bool temp_bool = RegisterOptions(&AddOptions);
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
        for (size_t i = 0; i < seed.length(); ++i) {
            //DebugLogger() << "hash value: " << hash_value << " char: " << static_cast<int>(seed[i]);
            hash_value += (seed[i] * 61);
            hash_value %= 191;
        }
        DebugLogger() << "final hash value: " << hash_value
                      << " and returning: " << hash_value % static_cast<int>(enum_vals_count)
                      << " from 0 to " << static_cast<int>(enum_vals_count) - 1;
        return hash_value % static_cast<int>(enum_vals_count);
    }

    static char alphanum[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
}

GalaxySetupData::GalaxySetupData() :
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

GalaxySetupData::GalaxySetupData(GalaxySetupData&& base) :
    m_seed(std::move(base.m_seed)),
    m_size(base.m_size),
    m_shape(base.m_shape),
    m_age(base.m_age),
    m_starlane_freq(base.m_starlane_freq),
    m_planet_density(base.m_planet_density),
    m_specials_freq(base.m_specials_freq),
    m_monster_freq(base.m_monster_freq),
    m_native_freq(base.m_native_freq),
    m_ai_aggr(base.m_ai_aggr),
    m_game_rules(std::move(base.m_game_rules)),
    m_game_uid(std::move(base.m_game_uid))
{ SetSeed(m_seed); }

const std::string& GalaxySetupData::GetSeed() const
{ return m_seed; }

int GalaxySetupData::GetSize() const
{ return m_size; }

Shape GalaxySetupData::GetShape() const {
    if (m_shape != RANDOM)
        return m_shape;
    size_t num_shapes = static_cast<size_t>(GALAXY_SHAPES) - 1; // -1 so that RANDOM isn't counted
    return static_cast<Shape>(GetIdx(num_shapes, m_seed + "shape"));
}

GalaxySetupOption GalaxySetupData::GetAge() const {
    if (m_age != GALAXY_SETUP_RANDOM)
        return m_age;
    return static_cast<GalaxySetupOption>(GetIdx(3, m_seed + "age") + 1);       // need range 1-3 for age
}

GalaxySetupOption GalaxySetupData::GetStarlaneFreq() const {
    if (m_starlane_freq != GALAXY_SETUP_RANDOM)
        return m_starlane_freq;
    return static_cast<GalaxySetupOption>(GetIdx(3, m_seed + "lanes") + 1);     // need range 1-3 for starlane freq
}

GalaxySetupOption GalaxySetupData::GetPlanetDensity() const {
    if (m_planet_density != GALAXY_SETUP_RANDOM)
        return m_planet_density;
    return static_cast<GalaxySetupOption>(GetIdx(3, m_seed + "planets") + 1);   // need range 1-3 for planet density
}

GalaxySetupOption GalaxySetupData::GetSpecialsFreq() const {
    if (m_specials_freq != GALAXY_SETUP_RANDOM)
        return m_specials_freq;
    return static_cast<GalaxySetupOption>(GetIdx(4, m_seed + "specials"));      // need range 0-3 for planet density
}

GalaxySetupOption GalaxySetupData::GetMonsterFreq() const {
    if (m_monster_freq != GALAXY_SETUP_RANDOM)
        return m_monster_freq;
    return static_cast<GalaxySetupOption>(GetIdx(4, m_seed + "monsters"));      // need range 0-3 for monster frequency
}

GalaxySetupOption GalaxySetupData::GetNativeFreq() const {
    if (m_native_freq != GALAXY_SETUP_RANDOM)
        return m_native_freq;
    return static_cast<GalaxySetupOption>(GetIdx(4, m_seed + "natives"));       // need range 0-3 for native frequency
}

Aggression GalaxySetupData::GetAggression() const
{ return m_ai_aggr; }

const std::vector<std::pair<std::string, std::string>>& GalaxySetupData::GetGameRules() const
{ return m_game_rules; }

const std::string& GalaxySetupData::GetGameUID() const
{ return m_game_uid; }

void GalaxySetupData::SetSeed(const std::string& seed) {
    std::string new_seed = seed;
    if (new_seed.empty() || new_seed == "RANDOM") {
        ClockSeed();
        new_seed.clear();
        for (int i = 0; i < 8; ++i)
            new_seed += alphanum[RandSmallInt(0, (sizeof(alphanum) - 2))];
        DebugLogger() << "Set empty or requested random seed to " << new_seed;
    }
    m_seed = std::move(new_seed);
}

void GalaxySetupData::SetGameUID(const std::string& game_uid)
{ m_game_uid = game_uid; }

/////////////////////////////////////////////////////
// PlayerSetupData
/////////////////////////////////////////////////////
bool operator==(const PlayerSetupData& lhs, const PlayerSetupData& rhs) {
    return  lhs.m_client_type == rhs.m_client_type &&
            lhs.m_empire_color == rhs.m_empire_color &&
            lhs.m_empire_name == rhs.m_empire_name &&
            lhs.m_player_name == rhs.m_player_name &&
            lhs.m_save_game_empire_id == rhs.m_save_game_empire_id &&
            lhs.m_starting_species_name == rhs.m_starting_species_name &&
            lhs.m_player_ready == rhs.m_player_ready;
}

bool operator!=(const PlayerSetupData& lhs, const PlayerSetupData& rhs)
{ return !(lhs == rhs); }


////////////////////////////////////////////////////
// MultiplayerLobbyData
/////////////////////////////////////////////////////
std::string MultiplayerLobbyData::Dump() const {
    std::stringstream stream;
    for (const std::pair<int, PlayerSetupData>& psd : m_players) {
        stream << psd.first << ": " << (psd.second.m_player_name.empty() ? "NO NAME" : psd.second.m_player_name) << "  ";
        if (psd.second.m_client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            stream << "AI PLAYER";
        else if (psd.second.m_client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
            stream << "HUMAN PLAYER";
        else if (psd.second.m_client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
            stream << "HUMAN OBSERVER";
        else if (psd.second.m_client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
            stream << "HUMAN MODERATOR";
        else
            stream << "UNKNOWN CLIENT TPYE";
        stream << "  " << (psd.second.m_empire_name.empty() ? "NO EMPIRE NAME" : psd.second.m_empire_name) << std::endl;
    }
    return stream.str();
}

