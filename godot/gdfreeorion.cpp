#include "gdfreeorion.h"
#include <chrono>
#include <atomic>
#include <exception>
#include <memory>
#include <sstream>
#include <codecvt>

#include <OS.hpp>

#include "../util/Directories.h"
#include "../util/GameRules.h"
#include "../util/OptionsDB.h"
#include "../util/Version.h"
#include "../universe/System.h"
#include "../universe/Fleet.h"
#include "../client/ClientNetworking.h"
#include "../combat/CombatLogManager.h"

#include "GodotClientApp.h"
#include "OptionsDB.h"
#include "GodotSystem.h"
#include "GodotFleet.h"

using namespace godot;

std::atomic_bool quit(false);

void GDFreeOrion::network_thread() {
    while(!quit) {
        if (auto msg = this->app->Networking().GetMessage()) {
            this->handle_message(std::move(*msg));
        } else {
            OS::get_singleton()->delay_msec(20);
        }
    }
}

void GDFreeOrion::handle_message(Message&& msg) {
    try {
        switch (msg.Type()) {
        case Message::MessageType::AUTH_REQUEST: {
            std::string player_name;
            std::string auth;
            ExtractAuthRequestMessageData(msg, player_name, auth);
            emit_signal("auth_request", String(player_name.c_str()), String(auth.c_str()));
            break;
        }
        case Message::MessageType::SET_AUTH_ROLES: {
            ExtractSetAuthorizationRolesMessage(msg, app->Networking().AuthorizationRoles());
            break;
        }
        case Message::MessageType::JOIN_GAME: {
            int player_id;
            boost::uuids::uuid cookie;
            ExtractJoinAckMessageData(msg, player_id, cookie);
            app->Networking().SetPlayerID(player_id);
            break;
        }
        case Message::MessageType::HOST_ID: {
            const std::string& text = msg.Text();
            int host_id = Networking::INVALID_PLAYER_ID;
            try {
                host_id = boost::lexical_cast<int>(text);
            } catch (const boost::bad_lexical_cast& ex) {
                ErrorLogger() << "GDFreeOrion::handle_message(HOST_ID) could not convert \"" << text << "\" to host id";
            }

            app->Networking().SetHostPlayerID(host_id);
            break;
        }
        case Message::MessageType::PLAYER_STATUS: {
            Message::PlayerStatus status;
            int about_empire_id;
            ExtractPlayerStatusMessageData(msg, status, about_empire_id);
            emit_signal("empire_status", static_cast<int>(status), about_empire_id);
            break;
        }
        case Message::MessageType::HOST_SP_GAME: {
            try {
                int host_id = boost::lexical_cast<int>(msg.Text());

                app->Networking().SetPlayerID(host_id);
                app->Networking().SetHostPlayerID(host_id);
            } catch (const boost::bad_lexical_cast& ex) {
                ErrorLogger() << "GDFreeOrion::handle_message(HOST_ID) Host id " << msg.Text() << " is not a number: " << ex.what();
            }
            break;
        }
        case Message::MessageType::GAME_START: {
            bool loaded_game_data;
            bool ui_data_available;
            SaveGameUIData ui_data;
            bool save_state_string_available;
            std::string save_state_string; // ignored - used by AI but not by human client
            OrderSet orders;
            bool single_player_game = false;
            int empire_id = ALL_EMPIRES;
            int current_turn = INVALID_GAME_TURN;
            app->Orders().Reset();
            try {
                ExtractGameStartMessageData(msg,                 single_player_game,             empire_id,
                                            current_turn,        Empires(),                      GetUniverse(),
                                            GetSpeciesManager(), GetCombatLogManager(),          GetSupplyManager(),
                                            app->Players(),      app->Orders(),              loaded_game_data,
                                            ui_data_available,   ui_data,                        save_state_string_available,
                                            save_state_string,   app->GetGalaxySetupData());
            } catch (...) {
                return;
            }

            DebugLogger() << "Extracted GameStart message for turn: " << current_turn << " with empire: " << empire_id;

            app->SetSinglePlayerGame(single_player_game);
            app->SetEmpireID(empire_id);
            app->SetCurrentTurn(current_turn);

            GetGameRules().SetFromStrings(app->GetGalaxySetupData().GetGameRules());

            bool is_new_game = !(loaded_game_data && ui_data_available);
            emit_signal("start_game", is_new_game);
            break;
        }
        default:
            std::ostringstream stream;
            stream << msg.Type();
            emit_signal("ping", String(stream.str().c_str()));
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "GDFreeOrion::handle_message : Exception while reacting to message of type \""
                      << msg.Type() << "\". what: " << e.what();
    }
}

void GDFreeOrion::_register_methods() {
    register_method("_exit_tree", &GDFreeOrion::_exit_tree);
    register_method("_new_single_player_game", &GDFreeOrion::_new_single_player_game);
    register_method("_get_systems", &GDFreeOrion::_get_systems);
    register_method("_get_fleets", &GDFreeOrion::_get_fleets);
    register_method("network_thread", &GDFreeOrion::network_thread);
    register_signal<GDFreeOrion>("ping", "message", GODOT_VARIANT_TYPE_STRING);
    register_signal<GDFreeOrion>("auth_request", "player_name", GODOT_VARIANT_TYPE_STRING, "auth", GODOT_VARIANT_TYPE_STRING);
    register_signal<GDFreeOrion>("empire_status", "status", GODOT_VARIANT_TYPE_INT, "about_empire_id", GODOT_VARIANT_TYPE_INT);
    register_signal<GDFreeOrion>("start_game", "is_new_game", GODOT_VARIANT_TYPE_BOOL);
    register_property<GDFreeOrion, godot::OptionsDB*>("optionsDB",
        &GDFreeOrion::set_options,
        &GDFreeOrion::get_options,
        nullptr);
    register_property<GDFreeOrion, GodotNetworking*>("networking",
        &GDFreeOrion::set_networking,
        &GDFreeOrion::get_networking,
        nullptr);
}

GDFreeOrion::GDFreeOrion() {
}

GDFreeOrion::~GDFreeOrion() {
    // add your cleanup here
}

#if defined(FREEORION_ANDROID)
const godot_gdnative_ext_android_api_struct* GDFreeOrion::s_android_api_struct = nullptr;

void GDFreeOrion::set_android_api_struct(const godot_gdnative_ext_android_api_struct *android_api_struct)
{ s_android_api_struct = android_api_struct; }
#endif

void GDFreeOrion::_init() {
#if defined(FREEORION_ANDROID)
    SetAndroidEnvironment(s_android_api_struct->godot_android_get_env(),
                          s_android_api_struct->godot_android_get_activity());
#endif

    InitDirs(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(OS::get_singleton()->get_executable_path().unicode_str()));

    GetOptionsDB().SetFromFile(GetConfigPath(), FreeOrionVersionString());
    GetOptionsDB().SetFromFile(GetPersistentConfigPath());

#if !defined(FREEORION_ANDROID)
    std::vector<std::string> args;
    const PoolStringArray wargs = OS::get_singleton()->get_cmdline_args();
    const PoolStringArray::Read wargs_read = wargs.read();
    for (int i = 0; i < wargs.size(); ++ i) {
        args.emplace_back(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(wargs_read[i].unicode_str()));
    }

    // override previously-saved and default options with command line parameters and flags
    GetOptionsDB().SetFromCommandLine(args);
#endif

    CompleteXDGMigration();

#if !defined(FREEORION_ANDROID)
    // Handle the case where the resource.path does not exist anymore
    // gracefully by resetting it to the standard path into the
    // application bundle.  This may happen if a previous installed
    // version of FreeOrion was residing in a different directory.
    if (!boost::filesystem::exists(GetResourceDir()) ||
        !boost::filesystem::exists(GetResourceDir() / "credits.xml") ||
        !boost::filesystem::exists(GetResourceDir() / "data" / "art" / "misc" / "missing.png"))
    {
        DebugLogger() << "Resources directory " << PathToString(GetResourceDir()) << " from config.xml missing or does not contain expected files. Resetting to default.";

        // GetOptionsDB().Set<std::string>("resource.path", "");
        GetOptionsDB().Set<std::string>("resource.path", PathToString(boost::filesystem::canonical("../../default"))); // Temporary default for Godot client prototype development

        // double-check that resetting actually fixed things...
        if (!boost::filesystem::exists(GetResourceDir()) ||
            !boost::filesystem::exists(GetResourceDir() / "credits.xml") ||
            !boost::filesystem::exists(GetResourceDir() / "data" / "art" / "misc" / "missing.png"))
        {
            DebugLogger() << "Default Resources directory missing or does not contain expected files. Cannot start game.";
            throw std::runtime_error("Unable to load game resources at default location: " +
                                     PathToString(GetResourceDir()) + " : Install may be broken.");
        }
    }
#endif

    // initialize any variables here
    app = std::make_unique<GodotClientApp>();
    optionsDB = godot::OptionsDB::_new();
    networking = GodotNetworking::_new();
}

void GDFreeOrion::_process(float delta) {
}

void GDFreeOrion::_exit_tree() {
    quit = true;
    app.reset();

    networking->free();
    optionsDB->free();

    ShutdownLoggingSystemFileSink();
}

void GDFreeOrion::_new_single_player_game(bool quickstart) {
#ifdef FREEORION_ANDROID
    Godot::print(String("No single player game supported"));
#else
    app->NewSinglePlayerGame(quickstart);
#endif
}

godot::OptionsDB* GDFreeOrion::get_options() const
{ return optionsDB; }

void GDFreeOrion::set_options(godot::OptionsDB* ptr) {
    // ignore it
    Godot::print(String("Ignore setting options"));
}

GodotNetworking* GDFreeOrion::get_networking() const
{ return networking; }

void GDFreeOrion::set_networking(GodotNetworking* ptr) {
    // ignore it
    Godot::print(String("Ignore setting networking"));
}

Dictionary GDFreeOrion::_get_systems() const {
    Dictionary systems;
    for (const auto& sys : Objects().all<System>()) {
        systems[sys->ID()] = GodotSystem::wrap(sys);
    }
    return systems;
}

Dictionary GDFreeOrion::_get_fleets() const {
    Dictionary fleets;
    for (const auto& fleet : Objects().all<Fleet>()) {
        fleets[fleet->ID()] = GodotFleet::wrap(fleet);
    }
    return fleets;
}

