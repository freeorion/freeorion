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
