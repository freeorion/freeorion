#include "GodotNetworking.h"

#include <codecvt>

#include <boost/uuid/nil_generator.hpp>

#include "../util/Logger.h"
#include "../client/ClientNetworking.h"

#include "GodotClientApp.h"

using namespace godot;

void GodotNetworking::_register_methods() {
    register_method("_is_connected", &GodotNetworking::_is_connected);
    register_method("_connect_to_server", &GodotNetworking::_connect_to_server);
}

GodotNetworking::GodotNetworking()
{}

GodotNetworking::~GodotNetworking()
{}

void GodotNetworking::_init()
{}

bool GodotNetworking::_is_connected() const
{
    const auto* app = GodotClientApp::GetApp();
    if (app) {
        return app->Networking().IsConnected();
    } else {
        ErrorLogger() << "App uninitialized";
    }
    return false;
}

bool GodotNetworking::_connect_to_server(String dest, String player_name)
{
    auto* app = GodotClientApp::GetApp();
    if (app) {
        std::string dest8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(dest.unicode_str());
        if (app->Networking().ConnectToServer(dest8)) {
            std::string player_name8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(player_name.unicode_str());
            app->Networking().SendMessage(JoinGameMessage(player_name8, Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER, boost::uuids::nil_uuid()));
            return true;
        }
    } else {
        ErrorLogger() << "App uninitialized";
    }
    return false;
}

