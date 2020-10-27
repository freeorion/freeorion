
#include "GodotClientApp.h"

#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/LoggerWithOptionsDB.h"
#include "../util/OptionsDB.h"
#include "../util/Version.h"

GodotClientApp::GodotClientApp() {
    if (GetOptionsDB().Get<std::string>("log-file").empty()) {
        const std::string HUMAN_CLIENT_LOG_FILENAME((GetUserDataDir() / "freeorion-godot.log").string());
        GetOptionsDB().Set("log-file", HUMAN_CLIENT_LOG_FILENAME);
    }

    InitLoggingSystem(GetOptionsDB().Get<std::string>("log-file"), "Godot");
    InitLoggingOptionsDBSystem();

    InfoLogger() << FreeOrionVersionString();

    LogDependencyVersions();

    StartBackgroundParsing();
}

GodotClientApp::~GodotClientApp() {

}

int GodotClientApp::EffectsProcessingThreads() const
{ return GetOptionsDB().Get<int>("effects.ui.threads"); }

