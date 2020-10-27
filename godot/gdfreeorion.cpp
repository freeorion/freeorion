#include "gdfreeorion.h"
#include <chrono>
#include <atomic>

#include "../util/Directories.h"
#include "../util/OptionsDB.h"
#include "../util/Version.h"

#include "GodotClientApp.h"

using namespace godot;

std::atomic_bool quit(false);

void do_the_ping(Node* n) {
    while(!quit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        n->emit_signal("ping");
    }
}

void GDFreeOrion::_register_methods() {
    // register_method("_process", &GDCppTest::_process);
    register_method("_exit_tree", &GDFreeOrion::_exit_tree);
    register_signal<GDFreeOrion>((char *)"ping");
}

GDFreeOrion::GDFreeOrion() {
}

GDFreeOrion::~GDFreeOrion() {
    // add your cleanup here
}

void GDFreeOrion::_init() {
    InitDirs("");

    GetOptionsDB().SetFromFile(GetConfigPath(), FreeOrionVersionString());
    GetOptionsDB().SetFromFile(GetPersistentConfigPath());

    CompleteXDGMigration();

    // Handle the case where the resource.path does not exist anymore
    // gracefully by resetting it to the standard path into the
    // application bundle.  This may happen if a previous installed
    // version of FreeOrion was residing in a different directory.
    if (!boost::filesystem::exists(GetResourceDir()) ||
        !boost::filesystem::exists(GetResourceDir() / "credits.xml") ||
        !boost::filesystem::exists(GetResourceDir() / "data" / "art" / "misc" / "missing.png"))
    {
        DebugLogger() << "Resources directory from config.xml missing or does not contain expected files. Resetting to default.";

        GetOptionsDB().Set<std::string>("resource.path", "");

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

    // initialize any variables here
    time_passed = 0.0;
    t = std::thread(do_the_ping, this);
    app = std::make_unique<GodotClientApp>();
}

void GDFreeOrion::_process(float delta) {
    time_passed += delta;
    if(time_passed > 2.0) {
        emit_signal("ping");
        time_passed = 0.0;
    }
}

void GDFreeOrion::_exit_tree() {
    quit = true;
    t.join();
    app.reset();

    ShutdownLoggingSystemFileSink();
}
