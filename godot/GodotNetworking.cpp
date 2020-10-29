#include "GodotNetworking.h"

#include <codecvt>

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

bool GodotNetworking::_connect_to_server(String dest)
{
    auto* app = GodotClientApp::GetApp();
    if (app) {
        std::string dest8 = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.to_bytes(dest.unicode_str());
        return app->Networking().ConnectToServer(dest8);
    } else {
        ErrorLogger() << "App uninitialized";
    }
    return false;
}

