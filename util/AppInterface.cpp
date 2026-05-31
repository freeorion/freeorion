#include "AppInterface.h"

#include "../universe/Universe.h"

#include <fstream>

#include <exception>
#include <future>
#include <stdexcept>


IApp* IApp::s_app = nullptr;

IApp::IApp() {
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of Application");

    s_app = this;
}

IApp::~IApp()
{ s_app = nullptr; }


int IApp::MAX_AI_PLAYERS() noexcept {
    // This is not just a constant to avoid the static initialization
    // order fiasco, because it is used in more than one compilation
    // unit during static initialization, albeit a the moment in two
    // different threads.
    static constexpr int max_number_AIs = 40;
    return max_number_AIs;
}

ObjectMap& IApp::Objects() {
    return GetUniverse().Objects();
}
