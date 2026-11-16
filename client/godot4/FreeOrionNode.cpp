#include "FreeOrionNode.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

#include <boost/format.hpp>

#include <sstream>

#include "GodotClientApp.h"
#include "GodotI18n.h"

#include "../ClientNetworking.h"
#include "../../combat/CombatLogManager.h"
#include "../../util/Directories.h"
#include "../../util/GameRules.h"
#include "../../util/i18n.h"
#include "../../util/OptionsDB.h"
#include "../../util/Version.h"
#include "../../util/XMLDoc.h"

std::atomic_bool quit(false);

namespace {
    auto escape_bbcode(const std::string& text) {
        std::string escaped;
        escaped.reserve(text.size());
        for (char c : text) {
            if (c == '[')
                escaped += "[lb]";
            else if (c == ']')
                escaped += "[rb]";
            else
                escaped += c;
        }
        return escaped;
    }
}

void FreeOrionNode::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("get_version"), &FreeOrionNode::get_version);
    godot::ClassDB::bind_method(godot::D_METHOD("get_user_data_dir"), &FreeOrionNode::get_user_data_dir);
    godot::ClassDB::bind_method(godot::D_METHOD("get_user_config_dir"), &FreeOrionNode::get_user_config_dir);
    godot::ClassDB::bind_method(godot::D_METHOD("get_current_turn"), &FreeOrionNode::get_current_turn);
    godot::ClassDB::bind_method(godot::D_METHOD("get_single_player_game"), &FreeOrionNode::get_single_player_game);
    godot::ClassDB::bind_method(godot::D_METHOD("get_credits_bbcode"), &FreeOrionNode::get_credits_bbcode);
    godot::ClassDB::bind_method(godot::D_METHOD("network_thread"), &FreeOrionNode::network_thread);
    godot::ClassDB::bind_method(godot::D_METHOD("parsing_thread"), &FreeOrionNode::parsing_thread);
    godot::ClassDB::bind_method(godot::D_METHOD("start_network_thread"), &FreeOrionNode::start_network_thread);
    godot::ClassDB::bind_method(godot::D_METHOD("start_parsing_thread"), &FreeOrionNode::start_parsing_thread);
    godot::ClassDB::bind_method(godot::D_METHOD("new_single_player_game"), &FreeOrionNode::new_single_player_game);
    godot::ClassDB::bind_method(godot::D_METHOD("start_turn"), &FreeOrionNode::start_turn);


    ADD_SIGNAL(godot::MethodInfo("parsing_completed"));
    ADD_SIGNAL(godot::MethodInfo("start_game", godot::PropertyInfo(godot::Variant::BOOL, "is_new_game")));
    ADD_SIGNAL(godot::MethodInfo("turn_update"));
}

FreeOrionNode::FreeOrionNode()
{ }

FreeOrionNode::~FreeOrionNode()
{ }

void FreeOrionNode::_ready() {
    if (godot::Engine::get_singleton()->is_editor_hint())
        return;

    std::string executable_path = godot::OS::get_singleton()->get_executable_path().utf8().get_data();
#ifdef FREEORION_MACOSX
    InitDirs(executable_path, true);
#else
    InitDirs(executable_path);
#endif

#ifdef FREEORION_WIN32
    GetOptionsDB().Add<std::filesystem::path>("misc.server-local-binary.path", UserStringNop("OPTIONS_DB_FREEORIOND_PATH"),   GetBinDir() / "freeoriond.exe");
#else
    GetOptionsDB().Add<std::filesystem::path>("misc.server-local-binary.path", UserStringNop("OPTIONS_DB_FREEORIOND_PATH"),   GetBinDir() / "freeoriond");
#endif

    GetOptionsDB().SetFromFile(GetConfigPath(), FreeOrionVersionString());
    GetOptionsDB().SetFromFile(GetPersistentConfigPath());

    std::vector<std::string> args;
    args.emplace_back(std::move(executable_path));
    const godot::PackedStringArray wargs = godot::OS::get_singleton()->get_cmdline_args();
    for (const godot::String &warg : wargs) {
        const std::string arg = warg.utf8().get_data();
        // Exclude Godot's options
        if (arg != "-s" && arg.rfind("-g", 0) != 0) {
            InfoLogger() << "Command line " << arg;
            args.emplace_back(std::move(arg));
        }
    }
    const godot::PackedStringArray uwargs = godot::OS::get_singleton()->get_cmdline_user_args();
    for (const godot::String &uwarg : uwargs) {
        const std::string arg = uwarg.utf8().get_data();
        // Exclude Godot's options
        if (arg != "-s" && arg.rfind("-g", 0) != 0) {
            InfoLogger() << "User command line " << arg;
            args.emplace_back(std::move(arg));
        }
    }
    // override previously-saved and default options with command line parameters and flags
    GetOptionsDB().SetFromCommandLine(args);

    CompleteXDGMigration();

#if !defined(FREEORION_ANDROID)
    // Handle the case where the resource.path does not exist anymore
    // gracefully by resetting it to the standard path into the
    // application bundle.  This may happen if a previous installed
    // version of FreeOrion was residing in a different directory.
    if (!std::filesystem::exists(GetResourceDir()) ||
        !std::filesystem::exists(GetResourceDir() / "credits.xml") ||
        !std::filesystem::exists(GetResourceDir() / "data" / "art" / "misc" / "missing.png"))
    {
        DebugLogger() << "Resources directory " << PathToString(GetResourceDir()) << " from config.xml missing or does not contain expected files. Resetting to default.";

        GetOptionsDB().Set<std::filesystem::path>("resource.path", std::filesystem::canonical("../default")); // Temporary default for Godot client prototype development

        // double-check that resetting actually fixed things...
        if (!std::filesystem::exists(GetResourceDir()) ||
            !std::filesystem::exists(GetResourceDir() / "credits.xml") ||
            !std::filesystem::exists(GetResourceDir() / "data" / "art" / "misc" / "missing.png"))
        {
            DebugLogger() << "Default Resources directory missing or does not contain expected files. Cannot start game.";
            throw std::runtime_error("Unable to load game resources at default location: " +
                                     PathToString(GetResourceDir()) + " : Install may be broken.");
        }
    }
#endif

    m_app = std::make_unique<GodotClientApp>();

    if (godot::Window* window = get_window()) {
        window->set_auto_translate_mode(godot::Node::AUTO_TRANSLATE_MODE_DISABLED);
        window->set_title("FreeOrion " + get_version());
    }

    godot::String locale = godot::String(Language().c_str());
    godot::Ref<GodotI18n> translation;
    translation.instantiate();
    translation->set_locale(locale);
    godot::TranslationServer::get_singleton()->add_translation(translation);
    godot::TranslationServer::get_singleton()->set_locale(locale);

    m_parsing_thread = godot::Ref<godot::Thread>();
    m_parsing_thread.instantiate();

    m_network_thread = godot::Ref<godot::Thread>();
    m_network_thread.instantiate();
}

void FreeOrionNode::_exit_tree() {
    if (godot::Engine::get_singleton()->is_editor_hint())
        return;

    DebugLogger() << "FreeOrionNode::_exit_tree(): Stopping freeorion node";
    quit = true;
    m_network_thread->wait_to_finish();
    m_parsing_thread->wait_to_finish();
    m_app.reset();
    DebugLogger() << "FreeOrionNode::_exit_tree(): Freeorion node stopped";

    ShutdownLoggingSystemFileSink();
}

godot::String FreeOrionNode::get_version() const
{ return godot::String(FreeOrionVersionString().c_str()); }

godot::String FreeOrionNode::get_user_data_dir() const
{ return godot::String(GetUserDataDir().native().c_str()); }

godot::String FreeOrionNode::get_user_config_dir() const
{ return godot::String(GetUserConfigDir().native().c_str()); }

int FreeOrionNode::get_current_turn() const
{ return m_app ? m_app->CurrentTurn() : INVALID_GAME_TURN; }

bool FreeOrionNode::get_single_player_game() const
{ return m_app ? m_app->SinglePlayerGame() : false; }

godot::String FreeOrionNode::get_credits_bbcode() const {
    std::string file_contents;
    if (!ReadFile(GetResourceDir() / "credits.xml", file_contents))
        return {};

    XMLDoc doc;
    doc.ReadDoc(file_contents);

    if (!doc.root_node.ContainsChild("CREDITS"))
        return {};

    const auto& credits_node = doc.root_node.Child("CREDITS");

    std::ostringstream credits;
    credits << "[center]";

    auto group_format = boost::format("%1%\n\n");
    auto nick_format = boost::format(" [color=#999999](%1%)[/color]");
    auto task_format = boost::format(" - [color=#cccccc]%1%[/color]");
    auto resource_format = boost::format("%1% - [color=#999999]%2%[/color]\n%3% %4%%5%\n");
    auto source_format = boost::format(" - [color=#999999]%1%[/color]");
    auto note_format = boost::format("[color=#cccccc](%1%)[/color]\n");

    for (const XMLElement& group : credits_node.Children()) {
        if (0 == group.Tag().compare("GROUP")) {
            credits << group_format % escape_bbcode(group.Attribute("name"));
            for (const XMLElement& item : group.Children()) {
                if (0 == item.Tag().compare("PERSON")) {
                    if (item.HasAttribute("name"))
                        credits << escape_bbcode(item.Attribute("name"));
                    if (item.HasAttribute("nick") && !item.Attribute("nick").empty())
                        credits << nick_format % escape_bbcode(item.Attribute("nick"));
                    if (item.HasAttribute("task"))
                        credits << task_format % escape_bbcode(item.Attribute("task"));
                }

                if (0 == item.Tag().compare("RESOURCE")) {
                    credits << resource_format
                        % escape_bbcode(item.Attribute("author"))
                        % escape_bbcode(item.Attribute("title"))
                        % UserString("INTRO_CREDITS_LICENSE")
                        % escape_bbcode(item.Attribute("license"))
                        % ((item.HasAttribute("source"))
                            ? boost::str(source_format % escape_bbcode(item.Attribute("source")))
                            : std::string{});
                    if (item.HasAttribute("notes"))
                        credits << note_format % escape_bbcode(item.Attribute("notes"));
                }

                credits << "\n";
            }

            credits << "\n\n";
        }
    }

    credits << "[/center]";

    return godot::String::utf8(credits.str().c_str());
}

void FreeOrionNode::network_thread() {
    DebugLogger() << "FreeOrionNode::network_thread(): Freeorion networking started";
    while(!quit) {
        if (auto msg = this->m_app->Networking().GetMessage()) {
            this->HandleMessage(std::move(*msg));
        } else {
            godot::OS::get_singleton()->delay_msec(20);
        }
    }
    DebugLogger() << "FreeOrionNode::network_thread(): Freeorion networking stopped";
}

void FreeOrionNode::parsing_thread() {
    DebugLogger() << "FreeOrionNode::parsing_thread(): Freeorion parsing started";
    m_app->StartParsingContent();
    call_deferred("emit_signal", "parsing_completed");
    DebugLogger() << "FreeOrionNode::parsing_thread(): Freeorion parsing stopped";
}

void FreeOrionNode::start_network_thread()
{ m_network_thread->start(godot::Callable(this, "network_thread")); }

void FreeOrionNode::start_parsing_thread()
{ m_parsing_thread->start(godot::Callable(this, "parsing_thread")); }

void FreeOrionNode::new_single_player_game()
{ m_app->NewSinglePlayerGame(); }

void FreeOrionNode::start_turn() {
    SaveGameUIData ui_data;
    m_app->StartTurn(ui_data);
}

void FreeOrionNode::HandleMessage(Message&& msg) {
    try {
        switch (msg.Type()) {
        case Message::MessageType::SET_AUTH_ROLES: {
            ExtractSetAuthorizationRolesMessage(msg, m_app->Networking().AuthorizationRoles());
            break;
        }
        case Message::MessageType::HOST_SP_GAME: {
            try {
                int host_id = boost::lexical_cast<int>(msg.Text());

                m_app->Networking().SetPlayerID(host_id);
                m_app->Networking().SetHostPlayerID(host_id);
            } catch (const boost::bad_lexical_cast& ex) {
                ErrorLogger() << "FreeOrionNode::HandleMessage : (HOST_SP_GAME) Host id " << msg.Text() << " is not a number: " << ex.what();
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
            m_app->Orders().Reset();
            try {
                ExtractGameStartMessageData(msg,                 single_player_game,             empire_id,
                                            current_turn,        Empires(),                      GetUniverse(),
                                            GetSpeciesManager(), GetCombatLogManager(),          GetSupplyManager(),
                                            m_app->Players(),    m_app->Orders(),                loaded_game_data,
                                            ui_data_available,   ui_data,                        save_state_string_available,
                                            save_state_string,   m_app->GetGalaxySetupData());
            } catch (...) {
                return;
            }

            DebugLogger() << "Extracted GameStart message for turn: " << current_turn << " with empire: " << empire_id;

            m_app->SetSinglePlayerGame(single_player_game);
            m_app->SetEmpireID(empire_id);
            m_app->SetCurrentTurn(current_turn);

            GetGameRules().SetFromStrings(m_app->GetGalaxySetupData().GetGameRules());

            bool is_new_game = !(loaded_game_data && ui_data_available);
            m_app->StartGame(is_new_game);
            call_deferred("emit_signal", "start_game", is_new_game);

            if (m_app->AutoTurnsLeft() > 0) {
                SaveGameUIData ui_data;
                m_app->StartTurn(ui_data);
                m_app->DecAutoTurns();
            }

            break;
        }
        case Message::MessageType::TURN_PROGRESS: {
            Message::TurnProgressPhase phase_id;
            ExtractTurnProgressMessageData(msg, phase_id);
            m_app->HandleTurnPhaseUpdate(phase_id);
            break;
        }
        case Message::MessageType::TURN_PARTIAL_UPDATE: {
            ExtractTurnPartialUpdateMessageData(msg, m_app->EmpireID(), GetUniverse());
            break;
        }
        case Message::MessageType::TURN_UPDATE: {
            int current_turn = INVALID_GAME_TURN;
            m_app->Orders().Reset();
            ExtractTurnUpdateMessageData(msg,                   m_app->EmpireID(),   current_turn,
                                         Empires(),             GetUniverse(),       GetSpeciesManager(),
                                         GetCombatLogManager(), GetSupplyManager(),  m_app->Players());
            m_app->SetCurrentTurn(current_turn);
            call_deferred("emit_signal", "turn_update");

            if (m_app->AutoTurnsLeft() > 0) {
                SaveGameUIData ui_data;
                m_app->StartTurn(ui_data);
                m_app->DecAutoTurns();
            }
            break;
        }
        default:
            ErrorLogger() << "FreeOrionNode::HandleMessage : Not implemented reacting to message of type\""
                          << msg.Type() << "\".";
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "FreeOrionNode::HandleMessage : Exception while reacting to message of type \""
                      << msg.Type() << "\". what: " << e.what();
    }
}
