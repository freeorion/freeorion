
#include "GodotClientApp.h"

#include "../ClientNetworking.h"
#include "../../Empire/Empire.h"
#include "../../parse/PythonParser.h"
#include "../../util/Directories.h"
#include "../../util/GameRules.h"
#include "../../util/i18n.h"
#include "../../util/Logger.h"
#include "../../util/LoggerWithOptionsDB.h"
#include "../../util/OptionsDB.h"
#include "../../util/PythonCommon.h"
#include "../../util/Version.h"

namespace {
    // persistant between-executions game setup settings
    void AddOptions(OptionsDB& db) {
        db.Add("setup.empire.name",              UserStringNop("OPTIONS_DB_GAMESETUP_EMPIRE_NAME"),              std::string(""),            Validator<std::string>());
        db.Add("setup.player.name",              UserStringNop("OPTIONS_DB_GAMESETUP_PLAYER_NAME"),              std::string(""),            Validator<std::string>());
        db.Add("setup.empire.color.index",       UserStringNop("OPTIONS_DB_GAMESETUP_EMPIRE_COLOR"),             9,                          RangedValidator<int>(0, 100));
        db.Add("setup.initial.species",          UserStringNop("OPTIONS_DB_GAMESETUP_STARTING_SPECIES_NAME"),    std::string("SP_HUMAN"),    Validator<std::string>());
        db.Add("setup.multiplayer.player.name",  UserStringNop("OPTIONS_DB_MP_PLAYER_NAME"),     std::string(""),            Validator<std::string>());
        db.Add("setup.multiplayer.host.address", UserStringNop("OPTIONS_DB_MP_HOST_ADDRESS"),    std::string("localhost"),   Validator<std::string>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    /** Sets the default and current values for the string option @p option_name to @p option_value if initially empty */
    void SetEmptyStringDefaultOption(const std::string& option_name, const std::string& option_value) {
        OptionsDB& db = GetOptionsDB();
        if (db.Get<std::string>(option_name).empty()) {
            db.SetDefault(option_name, option_value);
            db.Set(option_name, option_value);
        }
    }

    /* Sets the value of options that need language-dependent default values.*/
    void SetStringtableDependentOptionDefaults() {
        SetEmptyStringDefaultOption("setup.empire.name", UserString("DEFAULT_EMPIRE_NAME"));
        std::string player_name = UserString("DEFAULT_PLAYER_NAME");
        SetEmptyStringDefaultOption("setup.player.name", player_name);
        SetEmptyStringDefaultOption("setup.multiplayer.player.name", player_name);
    }

    class LocalServerAlreadyRunningException : public std::runtime_error {
    public:
        LocalServerAlreadyRunningException() :
            std::runtime_error("LOCAL_SERVER_ALREADY_RUNNING_ERROR")
        {}
    };

    void ClearPreviousPendingSaves(std::queue<std::string>& pending_saves) {
        if (pending_saves.empty())
            return;
        WarnLogger() << "Clearing " << std::to_string(pending_saves.size()) << " pending save game request(s)";
        std::queue<std::string>().swap(pending_saves);
    }
}

GodotClientApp::GodotClientApp() {
    if (GetOptionsDB().Get<std::string>("log-file").empty()) {
        const std::string HUMAN_CLIENT_LOG_FILENAME((GetUserDataDir() / "freeorion-godot.log").string());
        GetOptionsDB().Set("log-file", HUMAN_CLIENT_LOG_FILENAME);
    }

    InitLoggingSystem(GetOptionsDB().Get<std::string>("log-file"), "Godot");
    InitLoggingOptionsDBSystem();

    InfoLogger() << FreeOrionVersionString();

    LogDependencyVersions();

    SetStringtableDependentOptionDefaults();

    // Start parsing content
    std::promise<void> barrier;
    std::future<void> barrier_future = barrier.get_future();
    std::thread background([this] (auto b) {
        DebugLogger() << "Started background parser thread";
        PythonCommon python;
        python.Initialize();
        StartBackgroundParsing(PythonParser(python, GetResourceDir() / "scripting"), std::move(b));
    }, std::move(barrier));
    background.detach();
    barrier_future.wait();
}

GodotClientApp::~GodotClientApp() {

}

int GodotClientApp::EffectsProcessingThreads() const
{ return GetOptionsDB().Get<int>("effects.ui.threads"); }

GodotClientApp* GodotClientApp::GetApp()
{ return static_cast<GodotClientApp*>(s_app); }

#ifndef FREEORION_ANDROID
void GodotClientApp::StartServer() {
    if (m_networking->PingLocalHostServer(std::chrono::milliseconds(100))) {
        ErrorLogger() << "Can't start local server because a server is already connecting at 127.0.0.0.";
        throw LocalServerAlreadyRunningException();
    }

    std::string SERVER_CLIENT_EXE = GetOptionsDB().Get<std::string>("misc.server-local-binary.path");
    DebugLogger() << "GodotClientApp::StartServer: " << SERVER_CLIENT_EXE;

#ifdef FREEORION_MACOSX
    // On OSX set environment variable DYLD_LIBRARY_PATH to python framework folder
    // bundled with app, so the dynamic linker uses the bundled python library.
    // Otherwise the dynamic linker will look for a correct python lib in system
    // paths, and if it can't find it, throw an error and terminate!
    // Setting environment variable here, spawned child processes will inherit it.
    const char* old_library_path = getenv("DYLD_LIBRARY_PATH");
    const auto library_path = (old_library_path != nullptr) ? (GetPythonHome().string() + ":" + old_library_path) : GetPythonHome().string();
    setenv("DYLD_LIBRARY_PATH", library_path.c_str(), 1);
#endif

    std::vector<std::string> args;
    std::string ai_config = GetOptionsDB().Get<std::string>("ai-config");
    std::string ai_path = GetOptionsDB().Get<std::string>("ai-path");
    args.emplace_back("\"" + SERVER_CLIENT_EXE + "\"");
    args.emplace_back("--resource.path");
    args.emplace_back("\"" + GetOptionsDB().Get<std::string>("resource.path") + "\"");

    auto force_log_level = GetOptionsDB().Get<std::string>("log-level");
    if (!force_log_level.empty()) {
        args.emplace_back("--log-level");
        args.emplace_back(GetOptionsDB().Get<std::string>("log-level"));
    }

    if (ai_path != GetOptionsDB().GetDefaultValueString("ai-path")) {
        args.emplace_back("--ai-path");
        args.emplace_back(ai_path);
        DebugLogger() << "ai-path set to '" << ai_path << "'";
    }
    if (!ai_config.empty()) {
        args.emplace_back("--ai-config");
        args.emplace_back(ai_config);
        DebugLogger() << "ai-config set to '" << ai_config << "'";
    } else {
        DebugLogger() << "ai-config not set.";
    }
    if (m_single_player_game) {
        args.emplace_back("--singleplayer");
        args.emplace_back("--skip-checksum");
    }
    DebugLogger() << "Launching server process with args: ";
    for (auto arg : args)
        DebugLogger() << arg;
    m_server_process = Process(SERVER_CLIENT_EXE, args);
    DebugLogger() << "... finished launching server process.";
}

void GodotClientApp::FreeServer() {
    m_server_process.Free();
    m_networking->SetPlayerID(Networking::INVALID_PLAYER_ID);
    m_networking->SetHostPlayerID(Networking::INVALID_PLAYER_ID);
    SetEmpireID(ALL_EMPIRES);
}

void GodotClientApp::NewSinglePlayerGame() {
    DebugLogger() << "GodotClientApp::NewSinglePlayerGame start";
    ClearPreviousPendingSaves(m_game_saves_in_progress);

    m_single_player_game = true;
    try {
        StartServer();
    } catch (const LocalServerAlreadyRunningException& err) {
        return;
    } catch (const std::runtime_error& err) {
        ErrorLogger() << "GodotClientApp::NewSinglePlayerGame : Couldn't start server.  Got error message: " << err.what();
        return;
    }

    auto game_rules = GetGameRules().GetRulesAsStrings();

    if (!m_networking->ConnectToLocalHostServer()) {
        DebugLogger() << "Not connected; returning to intro screen and showing timed out error";
        //ResetToIntro(true);
        //ClientUI::MessageBox(UserString("ERR_CONNECT_TIMED_OUT"), true);
        return;
    }

    SinglePlayerSetupData setup_data;
    setup_data.new_game = true;
    setup_data.filename.clear();  // not used for new game

    // get values stored in options from previous time game was run or
    // from just having run GalaxySetupWnd

    // GalaxySetupData
    setup_data.SetSeed(GetOptionsDB().Get<std::string>("setup.seed"));
    setup_data.size =             GetOptionsDB().Get<int>("setup.star.count");
    setup_data.shape =            GetOptionsDB().Get<Shape>("setup.galaxy.shape");
    setup_data.age =              GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.galaxy.age");
    setup_data.starlane_freq =    GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.starlane.frequency");
    setup_data.planet_density =   GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.planet.density");
    setup_data.specials_freq =    GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.specials.frequency");
    setup_data.monster_freq =     GetOptionsDB().Get<GalaxySetupOptionMonsterFreq>("setup.monster.frequency");
    setup_data.native_freq =      GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.native.frequency");
    setup_data.ai_aggr =          GetOptionsDB().Get<Aggression>("setup.ai.aggression");
    setup_data.game_rules =       game_rules;


    // SinglePlayerSetupData contains a map of PlayerSetupData, for
    // the human and AI players.  Need to compile this information
    // from the specified human options and number of requested AIs

    // Human player setup data
    PlayerSetupData human_player_setup_data;
    human_player_setup_data.player_name = GetOptionsDB().Get<std::string>("setup.player.name");
    human_player_setup_data.empire_name = GetOptionsDB().Get<std::string>("setup.empire.name");

    // DB stores index into array of available colours, so need to get that array to look up value of index.
    // if stored value is invalid, use a default colour
    const std::vector<EmpireColor>& empire_colours = EmpireColors();
    int colour_index = GetOptionsDB().Get<int>("setup.empire.color.index");
    if (colour_index >= 0 && colour_index < static_cast<int>(empire_colours.size()))
        human_player_setup_data.empire_color = empire_colours[colour_index];
    else
        human_player_setup_data.empire_color = {0, 255, 0, 0};

    human_player_setup_data.starting_species_name = GetOptionsDB().Get<std::string>("setup.initial.species");
    if (human_player_setup_data.starting_species_name == "1")
        human_player_setup_data.starting_species_name = "SP_HUMAN";   // kludge / bug workaround for bug with options storage and retreival.  Empty-string options are stored, but read in as "true" boolean, and converted to string equal to "1"

    if (human_player_setup_data.starting_species_name != "RANDOM" &&
        !GetSpeciesManager().GetSpecies(human_player_setup_data.starting_species_name))
    {
        const SpeciesManager& sm = GetSpeciesManager();
        if (sm.NumPlayableSpecies() < 1)
            human_player_setup_data.starting_species_name.clear();
        else
            human_player_setup_data.starting_species_name = sm.playable_begin()->first;
    }

    human_player_setup_data.save_game_empire_id = ALL_EMPIRES; // not used for new games
    human_player_setup_data.client_type = Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER;

    // add to setup data players
    setup_data.players.push_back(human_player_setup_data);

    // AI player setup data.  One entry for each requested AI
    int num_AIs = GetOptionsDB().Get<int>("setup.ai.player.count");
    for (int ai_i = 1; ai_i <= num_AIs; ++ai_i) {
        PlayerSetupData ai_setup_data;

        ai_setup_data.player_name = "AI_" + std::to_string(ai_i);
        ai_setup_data.empire_name.clear();                // leave blank, to be set by server in Universe::GenerateEmpires
        ai_setup_data.starting_species_name.clear();      // leave blank, to be set by server
        ai_setup_data.save_game_empire_id = ALL_EMPIRES;  // not used for new games
        ai_setup_data.client_type = Networking::ClientType::CLIENT_TYPE_AI_PLAYER;

        setup_data.players.push_back(ai_setup_data);
    }


    DebugLogger() << "Sending host SP setup message";
    m_networking->SendMessage(HostSPGameMessage(setup_data, DependencyVersions()));
    DebugLogger() << "GodotClientApp::NewSinglePlayerGame done";
}
#endif

bool GodotClientApp::SinglePlayerGame() const
{ return m_single_player_game; }

void GodotClientApp::SetSinglePlayerGame(bool sp/* = true*/)
{ m_single_player_game = sp; }



