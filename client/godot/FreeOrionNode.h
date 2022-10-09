#ifndef _FreeOrionNode_h_
#define _FreeOrionNode_h_

#include <memory>

#include <Godot.hpp>
#include <Node.hpp>
#include <String.hpp>
#include <Thread.hpp>

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

    void HandleMessage(Message&&); ///< Processes message in the networking thread

    void network_thread(); ///< Function called in a separate networking thread

    void start_network_thread(); ///< Starts separate networking thread, should be called after initialization

    void new_single_player_game(); ///< Starts new single player game

    godot::String get_version() const; ///< Returns FreeOrion version

    bool is_server_connected() const; ///< Returns if FreeOrion connected to server

    bool connect_to_server(godot::String dest); ///< Connects to \a dest server

    void join_game(godot::String player_name, int client_type); ///< Joins to connected server

    void auth_response(godot::String player_name, godot::String password); ///< Sends \a password to the server

    godot::Dictionary get_systems() const; ///< Returns Godot Dictionary with systems

    godot::Dictionary get_fleets() const; ///< Returns Godot Dictionary with fleets

    void send_chat_message(godot::String text); ///< Sends \a text to chat

    void options_commit(); ///< Commits options DB

    void options_set(godot::String option, godot::Variant value); ///< Sets options DB value

    std::unique_ptr<GodotClientApp> m_app;
    godot::Ref<godot::Thread> m_network_thread;

#if defined(FREEORION_ANDROID)
    static const godot_gdnative_ext_android_api_struct* s_android_api_struct;
#endif
};

#endif
