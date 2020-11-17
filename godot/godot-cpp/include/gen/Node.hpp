#ifndef GODOT_CPP_NODE_HPP
#define GODOT_CPP_NODE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Node.hpp"

#include "Object.hpp"
namespace godot {

class InputEvent;
class InputEventKey;
class Node;
class MultiplayerAPI;
class SceneTree;
class Viewport;

class Node : public Object {
	struct ___method_bindings {
		godot_method_bind *mb__enter_tree;
		godot_method_bind *mb__exit_tree;
		godot_method_bind *mb__get_configuration_warning;
		godot_method_bind *mb__get_editor_description;
		godot_method_bind *mb__get_import_path;
		godot_method_bind *mb__input;
		godot_method_bind *mb__physics_process;
		godot_method_bind *mb__process;
		godot_method_bind *mb__ready;
		godot_method_bind *mb__set_editor_description;
		godot_method_bind *mb__set_import_path;
		godot_method_bind *mb__unhandled_input;
		godot_method_bind *mb__unhandled_key_input;
		godot_method_bind *mb_add_child;
		godot_method_bind *mb_add_child_below_node;
		godot_method_bind *mb_add_to_group;
		godot_method_bind *mb_can_process;
		godot_method_bind *mb_duplicate;
		godot_method_bind *mb_find_node;
		godot_method_bind *mb_find_parent;
		godot_method_bind *mb_get_child;
		godot_method_bind *mb_get_child_count;
		godot_method_bind *mb_get_children;
		godot_method_bind *mb_get_custom_multiplayer;
		godot_method_bind *mb_get_filename;
		godot_method_bind *mb_get_groups;
		godot_method_bind *mb_get_index;
		godot_method_bind *mb_get_multiplayer;
		godot_method_bind *mb_get_name;
		godot_method_bind *mb_get_network_master;
		godot_method_bind *mb_get_node;
		godot_method_bind *mb_get_node_and_resource;
		godot_method_bind *mb_get_node_or_null;
		godot_method_bind *mb_get_owner;
		godot_method_bind *mb_get_parent;
		godot_method_bind *mb_get_path;
		godot_method_bind *mb_get_path_to;
		godot_method_bind *mb_get_pause_mode;
		godot_method_bind *mb_get_physics_process_delta_time;
		godot_method_bind *mb_get_position_in_parent;
		godot_method_bind *mb_get_process_delta_time;
		godot_method_bind *mb_get_process_priority;
		godot_method_bind *mb_get_scene_instance_load_placeholder;
		godot_method_bind *mb_get_tree;
		godot_method_bind *mb_get_viewport;
		godot_method_bind *mb_has_node;
		godot_method_bind *mb_has_node_and_resource;
		godot_method_bind *mb_is_a_parent_of;
		godot_method_bind *mb_is_displayed_folded;
		godot_method_bind *mb_is_greater_than;
		godot_method_bind *mb_is_in_group;
		godot_method_bind *mb_is_inside_tree;
		godot_method_bind *mb_is_network_master;
		godot_method_bind *mb_is_physics_processing;
		godot_method_bind *mb_is_physics_processing_internal;
		godot_method_bind *mb_is_processing;
		godot_method_bind *mb_is_processing_input;
		godot_method_bind *mb_is_processing_internal;
		godot_method_bind *mb_is_processing_unhandled_input;
		godot_method_bind *mb_is_processing_unhandled_key_input;
		godot_method_bind *mb_move_child;
		godot_method_bind *mb_print_stray_nodes;
		godot_method_bind *mb_print_tree;
		godot_method_bind *mb_print_tree_pretty;
		godot_method_bind *mb_propagate_call;
		godot_method_bind *mb_propagate_notification;
		godot_method_bind *mb_queue_free;
		godot_method_bind *mb_raise;
		godot_method_bind *mb_remove_and_skip;
		godot_method_bind *mb_remove_child;
		godot_method_bind *mb_remove_from_group;
		godot_method_bind *mb_replace_by;
		godot_method_bind *mb_request_ready;
		godot_method_bind *mb_rpc;
		godot_method_bind *mb_rpc_config;
		godot_method_bind *mb_rpc_id;
		godot_method_bind *mb_rpc_unreliable;
		godot_method_bind *mb_rpc_unreliable_id;
		godot_method_bind *mb_rset;
		godot_method_bind *mb_rset_config;
		godot_method_bind *mb_rset_id;
		godot_method_bind *mb_rset_unreliable;
		godot_method_bind *mb_rset_unreliable_id;
		godot_method_bind *mb_set_custom_multiplayer;
		godot_method_bind *mb_set_display_folded;
		godot_method_bind *mb_set_filename;
		godot_method_bind *mb_set_name;
		godot_method_bind *mb_set_network_master;
		godot_method_bind *mb_set_owner;
		godot_method_bind *mb_set_pause_mode;
		godot_method_bind *mb_set_physics_process;
		godot_method_bind *mb_set_physics_process_internal;
		godot_method_bind *mb_set_process;
		godot_method_bind *mb_set_process_input;
		godot_method_bind *mb_set_process_internal;
		godot_method_bind *mb_set_process_priority;
		godot_method_bind *mb_set_process_unhandled_input;
		godot_method_bind *mb_set_process_unhandled_key_input;
		godot_method_bind *mb_set_scene_instance_load_placeholder;
		godot_method_bind *mb_update_configuration_warning;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Node"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum PauseMode {
		PAUSE_MODE_INHERIT = 0,
		PAUSE_MODE_STOP = 1,
		PAUSE_MODE_PROCESS = 2,
	};
	enum DuplicateFlags {
		DUPLICATE_SIGNALS = 1,
		DUPLICATE_GROUPS = 2,
		DUPLICATE_SCRIPTS = 4,
		DUPLICATE_USE_INSTANCING = 8,
	};

	// constants
	const static int NOTIFICATION_APP_PAUSED = 1015;
	const static int NOTIFICATION_APP_RESUMED = 1014;
	const static int NOTIFICATION_CRASH = 1012;
	const static int NOTIFICATION_DRAG_BEGIN = 21;
	const static int NOTIFICATION_DRAG_END = 22;
	const static int NOTIFICATION_ENTER_TREE = 10;
	const static int NOTIFICATION_EXIT_TREE = 11;
	const static int NOTIFICATION_INSTANCED = 20;
	const static int NOTIFICATION_INTERNAL_PHYSICS_PROCESS = 26;
	const static int NOTIFICATION_INTERNAL_PROCESS = 25;
	const static int NOTIFICATION_MOVED_IN_PARENT = 12;
	const static int NOTIFICATION_OS_IME_UPDATE = 1013;
	const static int NOTIFICATION_OS_MEMORY_WARNING = 1009;
	const static int NOTIFICATION_PARENTED = 18;
	const static int NOTIFICATION_PATH_CHANGED = 23;
	const static int NOTIFICATION_PAUSED = 14;
	const static int NOTIFICATION_PHYSICS_PROCESS = 16;
	const static int NOTIFICATION_PROCESS = 17;
	const static int NOTIFICATION_READY = 13;
	const static int NOTIFICATION_TRANSLATION_CHANGED = 1010;
	const static int NOTIFICATION_UNPARENTED = 19;
	const static int NOTIFICATION_UNPAUSED = 15;
	const static int NOTIFICATION_WM_ABOUT = 1011;
	const static int NOTIFICATION_WM_FOCUS_IN = 1004;
	const static int NOTIFICATION_WM_FOCUS_OUT = 1005;
	const static int NOTIFICATION_WM_GO_BACK_REQUEST = 1007;
	const static int NOTIFICATION_WM_MOUSE_ENTER = 1002;
	const static int NOTIFICATION_WM_MOUSE_EXIT = 1003;
	const static int NOTIFICATION_WM_QUIT_REQUEST = 1006;
	const static int NOTIFICATION_WM_UNFOCUS_REQUEST = 1008;


	static Node *_new();

	// methods
	void _enter_tree();
	void _exit_tree();
	String _get_configuration_warning();
	String _get_editor_description() const;
	NodePath _get_import_path() const;
	void _input(const Ref<InputEvent> event);
	void _physics_process(const real_t delta);
	void _process(const real_t delta);
	void _ready();
	void _set_editor_description(const String editor_description);
	void _set_import_path(const NodePath import_path);
	void _unhandled_input(const Ref<InputEvent> event);
	void _unhandled_key_input(const Ref<InputEventKey> event);
	void add_child(const Node *node, const bool legible_unique_name = false);
	void add_child_below_node(const Node *node, const Node *child_node, const bool legible_unique_name = false);
	void add_to_group(const String group, const bool persistent = false);
	bool can_process() const;
	Node *duplicate(const int64_t flags = 15) const;
	Node *find_node(const String mask, const bool recursive = true, const bool owned = true) const;
	Node *find_parent(const String mask) const;
	Node *get_child(const int64_t idx) const;
	int64_t get_child_count() const;
	Array get_children() const;
	Ref<MultiplayerAPI> get_custom_multiplayer() const;
	String get_filename() const;
	Array get_groups() const;
	int64_t get_index() const;
	Ref<MultiplayerAPI> get_multiplayer() const;
	String get_name() const;
	int64_t get_network_master() const;
	Node *get_node(const NodePath path) const;
	Array get_node_and_resource(const NodePath path);
	Node *get_node_or_null(const NodePath path) const;
	Node *get_owner() const;
	Node *get_parent() const;
	NodePath get_path() const;
	NodePath get_path_to(const Node *node) const;
	Node::PauseMode get_pause_mode() const;
	real_t get_physics_process_delta_time() const;
	int64_t get_position_in_parent() const;
	real_t get_process_delta_time() const;
	int64_t get_process_priority() const;
	bool get_scene_instance_load_placeholder() const;
	SceneTree *get_tree() const;
	Viewport *get_viewport() const;
	bool has_node(const NodePath path) const;
	bool has_node_and_resource(const NodePath path) const;
	bool is_a_parent_of(const Node *node) const;
	bool is_displayed_folded() const;
	bool is_greater_than(const Node *node) const;
	bool is_in_group(const String group) const;
	bool is_inside_tree() const;
	bool is_network_master() const;
	bool is_physics_processing() const;
	bool is_physics_processing_internal() const;
	bool is_processing() const;
	bool is_processing_input() const;
	bool is_processing_internal() const;
	bool is_processing_unhandled_input() const;
	bool is_processing_unhandled_key_input() const;
	void move_child(const Node *child_node, const int64_t to_position);
	void print_stray_nodes();
	void print_tree();
	void print_tree_pretty();
	void propagate_call(const String method, const Array args = Array(), const bool parent_first = false);
	void propagate_notification(const int64_t what);
	void queue_free();
	void raise();
	void remove_and_skip();
	void remove_child(const Node *node);
	void remove_from_group(const String group);
	void replace_by(const Node *node, const bool keep_data = false);
	void request_ready();
	Variant rpc(const String method, const Array& __var_args = Array());
	void rpc_config(const String method, const int64_t mode);
	Variant rpc_id(const int64_t peer_id, const String method, const Array& __var_args = Array());
	Variant rpc_unreliable(const String method, const Array& __var_args = Array());
	Variant rpc_unreliable_id(const int64_t peer_id, const String method, const Array& __var_args = Array());
	void rset(const String property, const Variant value);
	void rset_config(const String property, const int64_t mode);
	void rset_id(const int64_t peer_id, const String property, const Variant value);
	void rset_unreliable(const String property, const Variant value);
	void rset_unreliable_id(const int64_t peer_id, const String property, const Variant value);
	void set_custom_multiplayer(const Ref<MultiplayerAPI> api);
	void set_display_folded(const bool fold);
	void set_filename(const String filename);
	void set_name(const String name);
	void set_network_master(const int64_t id, const bool recursive = true);
	void set_owner(const Node *owner);
	void set_pause_mode(const int64_t mode);
	void set_physics_process(const bool enable);
	void set_physics_process_internal(const bool enable);
	void set_process(const bool enable);
	void set_process_input(const bool enable);
	void set_process_internal(const bool enable);
	void set_process_priority(const int64_t priority);
	void set_process_unhandled_input(const bool enable);
	void set_process_unhandled_key_input(const bool enable);
	void set_scene_instance_load_placeholder(const bool load_placeholder);
	void update_configuration_warning();
	template <class... Args> Variant rpc(const String method, Args... args){
		return rpc(method, Array::make(args...));
	}
	template <class... Args> Variant rpc_id(const int64_t peer_id, const String method, Args... args){
		return rpc_id(peer_id,method, Array::make(args...));
	}
	template <class... Args> Variant rpc_unreliable(const String method, Args... args){
		return rpc_unreliable(method, Array::make(args...));
	}
	template <class... Args> Variant rpc_unreliable_id(const int64_t peer_id, const String method, Args... args){
		return rpc_unreliable_id(peer_id,method, Array::make(args...));
	}

};

}

#endif