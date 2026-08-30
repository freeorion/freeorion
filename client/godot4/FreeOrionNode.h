#ifndef _FreeOrionNode_h_
#define _FreeOrionNode_h_

#include <memory>

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/thread.hpp>

class GodotClientApp;
class Message;

class FreeOrionNode : public godot::Node {
    GDCLASS(FreeOrionNode, Node)
public:
    FreeOrionNode();
    ~FreeOrionNode();

    void _ready() override;
    void _exit_tree() override;
protected:
    static void _bind_methods();
private:
    godot::String get_version() const; ///< Returns FreeOrion version

    godot::String get_user_data_dir() const; ///< Returns user data dir

    godot::String get_user_config_dir() const; ///< Returns user config dir

    void network_thread(); ///< Function called in a separate networking thread

    void parsing_thread(); ///< Function called in a separate parsing thread

    void start_network_thread(); ///< Starts separate networking thread, should be called after initialization

    void start_parsing_thread(); ///< Starts separate parsing thread, should be called after initialization

    void new_single_player_game(); ///< Starts new single player game

    void HandleMessage(Message&&); ///< Processes message in the networking thread

    std::unique_ptr<GodotClientApp> m_app;
    godot::Ref<godot::Thread> m_network_thread;
    godot::Ref<godot::Thread> m_parsing_thread;
};

#endif
