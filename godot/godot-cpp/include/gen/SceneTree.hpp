#ifndef GODOT_CPP_SCENETREE_HPP
#define GODOT_CPP_SCENETREE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "MainLoop.hpp"
namespace godot {

class Node;
class PackedScene;
class SceneTreeTimer;
class MultiplayerAPI;
class NetworkedMultiplayerPeer;
class Viewport;
class Object;

class SceneTree : public MainLoop {
	struct ___method_bindings {
		godot_method_bind *mb__change_scene;
		godot_method_bind *mb__connected_to_server;
		godot_method_bind *mb__connection_failed;
		godot_method_bind *mb__network_peer_connected;
		godot_method_bind *mb__network_peer_disconnected;
		godot_method_bind *mb__server_disconnected;
		godot_method_bind *mb_call_group;
		godot_method_bind *mb_call_group_flags;
		godot_method_bind *mb_change_scene;
		godot_method_bind *mb_change_scene_to;
		godot_method_bind *mb_create_timer;
		godot_method_bind *mb_get_current_scene;
		godot_method_bind *mb_get_edited_scene_root;
		godot_method_bind *mb_get_frame;
		godot_method_bind *mb_get_multiplayer;
		godot_method_bind *mb_get_network_connected_peers;
		godot_method_bind *mb_get_network_peer;
		godot_method_bind *mb_get_network_unique_id;
		godot_method_bind *mb_get_node_count;
		godot_method_bind *mb_get_nodes_in_group;
		godot_method_bind *mb_get_root;
		godot_method_bind *mb_get_rpc_sender_id;
		godot_method_bind *mb_has_group;
		godot_method_bind *mb_has_network_peer;
		godot_method_bind *mb_is_debugging_collisions_hint;
		godot_method_bind *mb_is_debugging_navigation_hint;
		godot_method_bind *mb_is_input_handled;
		godot_method_bind *mb_is_multiplayer_poll_enabled;
		godot_method_bind *mb_is_network_server;
		godot_method_bind *mb_is_paused;
		godot_method_bind *mb_is_refusing_new_network_connections;
		godot_method_bind *mb_is_using_font_oversampling;
		godot_method_bind *mb_notify_group;
		godot_method_bind *mb_notify_group_flags;
		godot_method_bind *mb_queue_delete;
		godot_method_bind *mb_quit;
		godot_method_bind *mb_reload_current_scene;
		godot_method_bind *mb_set_auto_accept_quit;
		godot_method_bind *mb_set_current_scene;
		godot_method_bind *mb_set_debug_collisions_hint;
		godot_method_bind *mb_set_debug_navigation_hint;
		godot_method_bind *mb_set_edited_scene_root;
		godot_method_bind *mb_set_group;
		godot_method_bind *mb_set_group_flags;
		godot_method_bind *mb_set_input_as_handled;
		godot_method_bind *mb_set_multiplayer;
		godot_method_bind *mb_set_multiplayer_poll_enabled;
		godot_method_bind *mb_set_network_peer;
		godot_method_bind *mb_set_pause;
		godot_method_bind *mb_set_quit_on_go_back;
		godot_method_bind *mb_set_refuse_new_network_connections;
		godot_method_bind *mb_set_screen_stretch;
		godot_method_bind *mb_set_use_font_oversampling;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SceneTree"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum StretchAspect {
		STRETCH_ASPECT_IGNORE = 0,
		STRETCH_ASPECT_KEEP = 1,
		STRETCH_ASPECT_KEEP_WIDTH = 2,
		STRETCH_ASPECT_KEEP_HEIGHT = 3,
		STRETCH_ASPECT_EXPAND = 4,
	};
	enum GroupCallFlags {
		GROUP_CALL_DEFAULT = 0,
		GROUP_CALL_REVERSE = 1,
		GROUP_CALL_REALTIME = 2,
		GROUP_CALL_UNIQUE = 4,
	};
	enum StretchMode {
		STRETCH_MODE_DISABLED = 0,
		STRETCH_MODE_2D = 1,
		STRETCH_MODE_VIEWPORT = 2,
	};

	// constants


	static SceneTree *_new();

	// methods
	void _change_scene(const Node *arg0);
	void _connected_to_server();
	void _connection_failed();
	void _network_peer_connected(const int64_t arg0);
	void _network_peer_disconnected(const int64_t arg0);
	void _server_disconnected();
	Variant call_group(const String group, const String method, const Array& __var_args = Array());
	Variant call_group_flags(const int64_t flags, const String group, const String method, const Array& __var_args = Array());
	Error change_scene(const String path);
	Error change_scene_to(const Ref<PackedScene> packed_scene);
	Ref<SceneTreeTimer> create_timer(const real_t time_sec, const bool pause_mode_process = true);
	Node *get_current_scene() const;
	Node *get_edited_scene_root() const;
	int64_t get_frame() const;
	Ref<MultiplayerAPI> get_multiplayer() const;
	PoolIntArray get_network_connected_peers() const;
	Ref<NetworkedMultiplayerPeer> get_network_peer() const;
	int64_t get_network_unique_id() const;
	int64_t get_node_count() const;
	Array get_nodes_in_group(const String group);
	Viewport *get_root() const;
	int64_t get_rpc_sender_id() const;
	bool has_group(const String name) const;
	bool has_network_peer() const;
	bool is_debugging_collisions_hint() const;
	bool is_debugging_navigation_hint() const;
	bool is_input_handled();
	bool is_multiplayer_poll_enabled() const;
	bool is_network_server() const;
	bool is_paused() const;
	bool is_refusing_new_network_connections() const;
	bool is_using_font_oversampling() const;
	void notify_group(const String group, const int64_t notification);
	void notify_group_flags(const int64_t call_flags, const String group, const int64_t notification);
	void queue_delete(const Object *obj);
	void quit(const int64_t exit_code = -1);
	Error reload_current_scene();
	void set_auto_accept_quit(const bool enabled);
	void set_current_scene(const Node *child_node);
	void set_debug_collisions_hint(const bool enable);
	void set_debug_navigation_hint(const bool enable);
	void set_edited_scene_root(const Node *scene);
	void set_group(const String group, const String property, const Variant value);
	void set_group_flags(const int64_t call_flags, const String group, const String property, const Variant value);
	void set_input_as_handled();
	void set_multiplayer(const Ref<MultiplayerAPI> multiplayer);
	void set_multiplayer_poll_enabled(const bool enabled);
	void set_network_peer(const Ref<NetworkedMultiplayerPeer> peer);
	void set_pause(const bool enable);
	void set_quit_on_go_back(const bool enabled);
	void set_refuse_new_network_connections(const bool refuse);
	void set_screen_stretch(const int64_t mode, const int64_t aspect, const Vector2 minsize, const real_t shrink = 1);
	void set_use_font_oversampling(const bool enable);
	template <class... Args> Variant call_group(const String group, const String method, Args... args){
		return call_group(group,method, Array::make(args...));
	}
	template <class... Args> Variant call_group_flags(const int64_t flags, const String group, const String method, Args... args){
		return call_group_flags(flags,group,method, Array::make(args...));
	}

};

}

#endif