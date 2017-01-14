#include "MultiplayerCommon.h"

#include "Directories.h"
#include "i18n.h"
#include "Logger.h"
#include "OptionsDB.h"
#include "Random.h"
#include "../universe/Fleet.h"
#include "../universe/Planet.h"
#include "../universe/System.h"

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif


#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>

#include <GG/utf8/checked.h>


const std::string MP_SAVE_FILE_EXTENSION = ".mps";
const std::string SP_SAVE_FILE_EXTENSION = ".sav";

namespace fs = boost::filesystem;

namespace {
    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add<std::string>("resource-dir",         UserStringNop("OPTIONS_DB_RESOURCE_DIR"),          PathString(GetRootDataDir() / "default"));
        db.Add<std::string>('S', "save-dir",        UserStringNop("OPTIONS_DB_SAVE_DIR"),              PathString(GetUserDataDir() / "save"));
        db.Add<std::string>("log-level",            UserStringNop("OPTIONS_DB_LOG_LEVEL"),             "DEBUG");
        db.Add<std::string>("stringtable-filename", UserStringNop("OPTIONS_DB_STRINGTABLE_FILENAME"),  PathString(GetRootDataDir() / "default" / "stringtables" / "en.txt"));
        db.Add("binary-serialization",              UserStringNop("OPTIONS_DB_BINARY_SERIALIZATION"),  false);

        // AI Testing options-- the following options are to facilitate AI testing and do not currently have an options page widget; 
        // they are intended to be changed via the command line and are not currently storable in the configuration file.
        db.Add<std::string>("ai-path",              UserStringNop("OPTIONS_DB_AI_FOLDER_PATH"),        "python/AI",     Validator<std::string>(), false);
        db.Add<std::string>("ai-config",            UserStringNop("OPTIONS_DB_AI_CONFIG"),             "",       Validator<std::string>(), false);
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
}

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


/////////////////////////////////////////////////////
// PlayerSetupData
/////////////////////////////////////////////////////
bool operator==(const PlayerSetupData& lhs, const PlayerSetupData& rhs) {
    return  lhs.m_client_type == rhs.m_client_type &&
            lhs.m_empire_color == rhs.m_empire_color &&
            lhs.m_empire_name == rhs.m_empire_name &&
            lhs.m_player_name == rhs.m_player_name &&
            lhs.m_save_game_empire_id == rhs.m_save_game_empire_id &&
            lhs.m_starting_species_name == rhs.m_starting_species_name;
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

