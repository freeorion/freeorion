#ifndef _FreeOrionNode_h_
#define _FreeOrionNode_h_

#include <memory>

#include <Godot.hpp>
#include <Node.hpp>
#include <String.hpp>

class Message;
class GodotClientApp;

/** Node for GDNative library of FreeOrion Godot client. */
class FreeOrionNode : public godot::Node {
    GODOT_CLASS(FreeOrionNode, godot::Node)

public:
    static void _register_methods(); ///< Registers GDNative methods, properties and signals

    FreeOrionNode();
    ~FreeOrionNode();

    /** Calls when Godot initializes Node.
      * Should be used instead of constructor. */
    void _init();

#if defined(FREEORION_ANDROID)
    static void set_android_api_struct(const godot_gdnative_ext_android_api_struct* android_api_struct);
#endif
private:
    /** Calls when Godot remove Node from scene at finalization.
      * Should be used instead of destructor. */
    void _exit_tree();

    void HandleMessage(Message&&); ///< Process message in the networking thread

    void network_thread(); ///< Function called in a separate networking thread

    void new_single_player_game(); ///< Starts new single player game

    godot::String get_version() const; ///< Returns FreeOrion version

    std::unique_ptr<GodotClientApp> app;

#if defined(FREEORION_ANDROID)
    static const godot_gdnative_ext_android_api_struct* s_android_api_struct;
#endif
};

#endif
