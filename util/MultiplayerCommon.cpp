#include "MultiplayerCommon.h"

#include "Directories.h"
#include "GameRules.h"
#include "i18n.h"
#include "LoggerWithOptionsDB.h"
#include "OptionsDB.h"
#include "Random.h"
#include "AppInterface.h"
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
                            Validator<std::string>(),                                                           false);
        // Default stringtable filename is deferred to i18n.cpp::InitStringtableFileName
        db.Add<std::string>("resource.stringtable.path",    UserStringNop("OPTIONS_DB_STRINGTABLE_FILENAME"),   "");
        db.Add("save.format.binary.enabled",                UserStringNop("OPTIONS_DB_BINARY_SERIALIZATION"),   false);
        db.Add("save.format.xml.zlib.enabled",              UserStringNop("OPTIONS_DB_XML_ZLIB_SERIALIZATION"), true);
        db.Add("save.auto.hostless.enabled",                UserStringNop("OPTIONS_DB_AUTOSAVE_HOSTLESS"),      true);
        db.Add("save.auto.hostless.each-player.enabled",    UserStringNop("OPTIONS_DB_AUTOSAVE_HOSTLESS_EACH_PLAYER"), false);
        db.Add<int>("save.auto.interval",                   UserStringNop("OPTIONS_DB_AUTOSAVE_INTERVAL"),      0);
        db.Add<std::string>("load",                         UserStringNop("OPTIONS_DB_LOAD"),                   "",                     Validator<std::string>(), false);
        db.Add("save.auto.exit.enabled",                    UserStringNop("OPTIONS_DB_AUTOSAVE_GAME_CLOSE"),    true);
        db.AddFlag('q', "quickstart",                       UserStringNop("OPTIONS_DB_QUICKSTART"),             false);

        // AI Testing options-- the following options are to facilitate AI testing and do not currently have an options page widget;
        // they are intended to be changed via the command line and are not currently storable in the configuration file.
        db.Add<std::string>("ai-path",                      UserStringNop("OPTIONS_DB_AI_FOLDER_PATH"),         "python/AI",
                            Validator<std::string>(),                                                           false);
        db.Add<std::string>("ai-config",                    UserStringNop("OPTIONS_DB_AI_CONFIG"),              "",
                            Validator<std::string>(),                                                           false);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    void AddRules(GameRules& rules) {
        rules.Add<int>("RULE_THRESHOLD_HUMAN_PLAYER_WIN", "RULE_THRESHOLD_HUMAN_PLAYER_WIN_DESC",
                       "MULTIPLAYER", 0, true,  RangedValidator<int>(0, 999));

        rules.Add<bool>("RULE_ONLY_ALLIANCE_WIN", "RULE_ONLY_ALLIANCE_WIN_DESC",
                       "MULTIPLAYER", true, true);

        rules.Add<bool>("RULE_ALLOW_CONCEDE", "RULE_ALLOW_CONCEDE_DESC",
                       "MULTIPLAYER", false, true);

        rules.Add<int>("RULE_CONCEDE_COLONIES_THRESHOLD", "RULE_CONCEDE_COLONIES_THRESHOLD_DESC",
                       "MULTIPLAYER", 1, true,  RangedValidator<int>(0, 9999));
    }
    bool temp_bool2 = RegisterGameRules(&AddRules);
}

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

bool operator!=(const PlayerSetupData& lhs, const PlayerSetupData& rhs)
{ return !(lhs == rhs); }


////////////////////////////////////////////////////
// MultiplayerLobbyData
/////////////////////////////////////////////////////
std::string MultiplayerLobbyData::Dump() const {
    std::stringstream stream;
    for (const std::pair<int, PlayerSetupData>& psd : players) {
        stream << psd.first << ": " << (psd.second.player_name.empty() ? "NO NAME" : psd.second.player_name) << "  ";
        if (psd.second.client_type == Networking::CLIENT_TYPE_AI_PLAYER)
            stream << "AI PLAYER";
        else if (psd.second.client_type == Networking::CLIENT_TYPE_HUMAN_PLAYER)
            stream << "HUMAN PLAYER";
        else if (psd.second.client_type == Networking::CLIENT_TYPE_HUMAN_OBSERVER)
            stream << "HUMAN OBSERVER";
        else if (psd.second.client_type == Networking::CLIENT_TYPE_HUMAN_MODERATOR)
            stream << "HUMAN MODERATOR";
        else
            stream << "UNKNOWN CLIENT TPYE";
        stream << "  " << (psd.second.empire_name.empty() ? "NO EMPIRE NAME" : psd.second.empire_name) << std::endl;
    }
    return stream.str();
}

