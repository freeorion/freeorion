#ifndef GDFREEORION_H
#define GDFREEORION_H

#include <Godot.hpp>
#include <Node.hpp>
#include <memory>
#include <thread>

#include "GodotNetworking.h"
#include "OptionsDB.h"
#include "../network/Message.h"

class GodotClientApp;

namespace godot {

class GDFreeOrion : public Node {
    GODOT_CLASS(GDFreeOrion, Node)

private:
    void network_thread();

    std::unique_ptr<GodotClientApp> app;
    godot::OptionsDB* optionsDB{nullptr};
    GodotNetworking* networking{nullptr};

#if defined(FREEORION_ANDROID)
    static const godot_gdnative_ext_android_api_struct* s_android_api_struct;
#endif
public:
    static void _register_methods();

    GDFreeOrion();
    ~GDFreeOrion();

    void _init(); // our initializer called by Godot

    void _process(float delta);
    void _exit_tree();
    void _new_single_player_game(bool quickstart);
    Dictionary _get_systems() const;
    Dictionary _get_fleets() const;

    godot::OptionsDB* get_options() const;
    void set_options(godot::OptionsDB* ptr);

    GodotNetworking* get_networking() const;
    void set_networking(GodotNetworking* ptr);

    void handle_message(Message&& msg); ///< handle messages in thread

#if defined(FREEORION_ANDROID)
    static void set_android_api_struct(const godot_gdnative_ext_android_api_struct* android_api_struct);
#endif
};

}

#endif

