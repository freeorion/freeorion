#include "gdcpptest.h"
#include <chrono>
#include <atomic>

using namespace godot;

std::atomic_bool quit(false);

void do_the_ping(Node* n) {
    while(!quit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        n->emit_signal("ping");
    }
}

void GDCppTest::_register_methods() {
    // register_method("_process", &GDCppTest::_process);
    register_method("_exit_tree", &GDCppTest::_exit_tree);
    register_signal<GDCppTest>((char *)"ping");
}

GDCppTest::GDCppTest() {
}

GDCppTest::~GDCppTest() {
    // add your cleanup here
}

void GDCppTest::_init() {
    // initialize any variables here
    time_passed = 0.0;
    t = std::thread(do_the_ping, this);
}

void GDCppTest::_process(float delta) {
    time_passed += delta;
    if(time_passed > 2.0) {
        emit_signal("ping");
        time_passed = 0.0;
    }
}

void GDCppTest::_exit_tree() {
    quit = true;
    t.join();
}
