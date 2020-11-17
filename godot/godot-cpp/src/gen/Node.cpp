#include "Node.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "InputEventKey.hpp"
#include "Node.hpp"
#include "MultiplayerAPI.hpp"
#include "SceneTree.hpp"
#include "Viewport.hpp"


namespace godot {


Node::___method_bindings Node::___mb = {};

void Node::___init_method_bindings() {
	___mb.mb__enter_tree = godot::api->godot_method_bind_get_method("Node", "_enter_tree");
	___mb.mb__exit_tree = godot::api->godot_method_bind_get_method("Node", "_exit_tree");
	___mb.mb__get_configuration_warning = godot::api->godot_method_bind_get_method("Node", "_get_configuration_warning");
	___mb.mb__get_editor_description = godot::api->godot_method_bind_get_method("Node", "_get_editor_description");
	___mb.mb__get_import_path = godot::api->godot_method_bind_get_method("Node", "_get_import_path");
	___mb.mb__input = godot::api->godot_method_bind_get_method("Node", "_input");
	___mb.mb__physics_process = godot::api->godot_method_bind_get_method("Node", "_physics_process");
	___mb.mb__process = godot::api->godot_method_bind_get_method("Node", "_process");
	___mb.mb__ready = godot::api->godot_method_bind_get_method("Node", "_ready");
	___mb.mb__set_editor_description = godot::api->godot_method_bind_get_method("Node", "_set_editor_description");
	___mb.mb__set_import_path = godot::api->godot_method_bind_get_method("Node", "_set_import_path");
	___mb.mb__unhandled_input = godot::api->godot_method_bind_get_method("Node", "_unhandled_input");
	___mb.mb__unhandled_key_input = godot::api->godot_method_bind_get_method("Node", "_unhandled_key_input");
	___mb.mb_add_child = godot::api->godot_method_bind_get_method("Node", "add_child");
	___mb.mb_add_child_below_node = godot::api->godot_method_bind_get_method("Node", "add_child_below_node");
	___mb.mb_add_to_group = godot::api->godot_method_bind_get_method("Node", "add_to_group");
	___mb.mb_can_process = godot::api->godot_method_bind_get_method("Node", "can_process");
	___mb.mb_duplicate = godot::api->godot_method_bind_get_method("Node", "duplicate");
	___mb.mb_find_node = godot::api->godot_method_bind_get_method("Node", "find_node");
	___mb.mb_find_parent = godot::api->godot_method_bind_get_method("Node", "find_parent");
	___mb.mb_get_child = godot::api->godot_method_bind_get_method("Node", "get_child");
	___mb.mb_get_child_count = godot::api->godot_method_bind_get_method("Node", "get_child_count");
	___mb.mb_get_children = godot::api->godot_method_bind_get_method("Node", "get_children");
	___mb.mb_get_custom_multiplayer = godot::api->godot_method_bind_get_method("Node", "get_custom_multiplayer");
	___mb.mb_get_filename = godot::api->godot_method_bind_get_method("Node", "get_filename");
	___mb.mb_get_groups = godot::api->godot_method_bind_get_method("Node", "get_groups");
	___mb.mb_get_index = godot::api->godot_method_bind_get_method("Node", "get_index");
	___mb.mb_get_multiplayer = godot::api->godot_method_bind_get_method("Node", "get_multiplayer");
	___mb.mb_get_name = godot::api->godot_method_bind_get_method("Node", "get_name");
	___mb.mb_get_network_master = godot::api->godot_method_bind_get_method("Node", "get_network_master");
	___mb.mb_get_node = godot::api->godot_method_bind_get_method("Node", "get_node");
	___mb.mb_get_node_and_resource = godot::api->godot_method_bind_get_method("Node", "get_node_and_resource");
	___mb.mb_get_node_or_null = godot::api->godot_method_bind_get_method("Node", "get_node_or_null");
	___mb.mb_get_owner = godot::api->godot_method_bind_get_method("Node", "get_owner");
	___mb.mb_get_parent = godot::api->godot_method_bind_get_method("Node", "get_parent");
	___mb.mb_get_path = godot::api->godot_method_bind_get_method("Node", "get_path");
	___mb.mb_get_path_to = godot::api->godot_method_bind_get_method("Node", "get_path_to");
	___mb.mb_get_pause_mode = godot::api->godot_method_bind_get_method("Node", "get_pause_mode");
	___mb.mb_get_physics_process_delta_time = godot::api->godot_method_bind_get_method("Node", "get_physics_process_delta_time");
	___mb.mb_get_position_in_parent = godot::api->godot_method_bind_get_method("Node", "get_position_in_parent");
	___mb.mb_get_process_delta_time = godot::api->godot_method_bind_get_method("Node", "get_process_delta_time");
	___mb.mb_get_process_priority = godot::api->godot_method_bind_get_method("Node", "get_process_priority");
	___mb.mb_get_scene_instance_load_placeholder = godot::api->godot_method_bind_get_method("Node", "get_scene_instance_load_placeholder");
	___mb.mb_get_tree = godot::api->godot_method_bind_get_method("Node", "get_tree");
	___mb.mb_get_viewport = godot::api->godot_method_bind_get_method("Node", "get_viewport");
	___mb.mb_has_node = godot::api->godot_method_bind_get_method("Node", "has_node");
	___mb.mb_has_node_and_resource = godot::api->godot_method_bind_get_method("Node", "has_node_and_resource");
	___mb.mb_is_a_parent_of = godot::api->godot_method_bind_get_method("Node", "is_a_parent_of");
	___mb.mb_is_displayed_folded = godot::api->godot_method_bind_get_method("Node", "is_displayed_folded");
	___mb.mb_is_greater_than = godot::api->godot_method_bind_get_method("Node", "is_greater_than");
	___mb.mb_is_in_group = godot::api->godot_method_bind_get_method("Node", "is_in_group");
	___mb.mb_is_inside_tree = godot::api->godot_method_bind_get_method("Node", "is_inside_tree");
	___mb.mb_is_network_master = godot::api->godot_method_bind_get_method("Node", "is_network_master");
	___mb.mb_is_physics_processing = godot::api->godot_method_bind_get_method("Node", "is_physics_processing");
	___mb.mb_is_physics_processing_internal = godot::api->godot_method_bind_get_method("Node", "is_physics_processing_internal");
	___mb.mb_is_processing = godot::api->godot_method_bind_get_method("Node", "is_processing");
	___mb.mb_is_processing_input = godot::api->godot_method_bind_get_method("Node", "is_processing_input");
	___mb.mb_is_processing_internal = godot::api->godot_method_bind_get_method("Node", "is_processing_internal");
	___mb.mb_is_processing_unhandled_input = godot::api->godot_method_bind_get_method("Node", "is_processing_unhandled_input");
	___mb.mb_is_processing_unhandled_key_input = godot::api->godot_method_bind_get_method("Node", "is_processing_unhandled_key_input");
	___mb.mb_move_child = godot::api->godot_method_bind_get_method("Node", "move_child");
	___mb.mb_print_stray_nodes = godot::api->godot_method_bind_get_method("Node", "print_stray_nodes");
	___mb.mb_print_tree = godot::api->godot_method_bind_get_method("Node", "print_tree");
	___mb.mb_print_tree_pretty = godot::api->godot_method_bind_get_method("Node", "print_tree_pretty");
	___mb.mb_propagate_call = godot::api->godot_method_bind_get_method("Node", "propagate_call");
	___mb.mb_propagate_notification = godot::api->godot_method_bind_get_method("Node", "propagate_notification");
	___mb.mb_queue_free = godot::api->godot_method_bind_get_method("Node", "queue_free");
	___mb.mb_raise = godot::api->godot_method_bind_get_method("Node", "raise");
	___mb.mb_remove_and_skip = godot::api->godot_method_bind_get_method("Node", "remove_and_skip");
	___mb.mb_remove_child = godot::api->godot_method_bind_get_method("Node", "remove_child");
	___mb.mb_remove_from_group = godot::api->godot_method_bind_get_method("Node", "remove_from_group");
	___mb.mb_replace_by = godot::api->godot_method_bind_get_method("Node", "replace_by");
	___mb.mb_request_ready = godot::api->godot_method_bind_get_method("Node", "request_ready");
	___mb.mb_rpc = godot::api->godot_method_bind_get_method("Node", "rpc");
	___mb.mb_rpc_config = godot::api->godot_method_bind_get_method("Node", "rpc_config");
	___mb.mb_rpc_id = godot::api->godot_method_bind_get_method("Node", "rpc_id");
	___mb.mb_rpc_unreliable = godot::api->godot_method_bind_get_method("Node", "rpc_unreliable");
	___mb.mb_rpc_unreliable_id = godot::api->godot_method_bind_get_method("Node", "rpc_unreliable_id");
	___mb.mb_rset = godot::api->godot_method_bind_get_method("Node", "rset");
	___mb.mb_rset_config = godot::api->godot_method_bind_get_method("Node", "rset_config");
	___mb.mb_rset_id = godot::api->godot_method_bind_get_method("Node", "rset_id");
	___mb.mb_rset_unreliable = godot::api->godot_method_bind_get_method("Node", "rset_unreliable");
	___mb.mb_rset_unreliable_id = godot::api->godot_method_bind_get_method("Node", "rset_unreliable_id");
	___mb.mb_set_custom_multiplayer = godot::api->godot_method_bind_get_method("Node", "set_custom_multiplayer");
	___mb.mb_set_display_folded = godot::api->godot_method_bind_get_method("Node", "set_display_folded");
	___mb.mb_set_filename = godot::api->godot_method_bind_get_method("Node", "set_filename");
	___mb.mb_set_name = godot::api->godot_method_bind_get_method("Node", "set_name");
	___mb.mb_set_network_master = godot::api->godot_method_bind_get_method("Node", "set_network_master");
	___mb.mb_set_owner = godot::api->godot_method_bind_get_method("Node", "set_owner");
	___mb.mb_set_pause_mode = godot::api->godot_method_bind_get_method("Node", "set_pause_mode");
	___mb.mb_set_physics_process = godot::api->godot_method_bind_get_method("Node", "set_physics_process");
	___mb.mb_set_physics_process_internal = godot::api->godot_method_bind_get_method("Node", "set_physics_process_internal");
	___mb.mb_set_process = godot::api->godot_method_bind_get_method("Node", "set_process");
	___mb.mb_set_process_input = godot::api->godot_method_bind_get_method("Node", "set_process_input");
	___mb.mb_set_process_internal = godot::api->godot_method_bind_get_method("Node", "set_process_internal");
	___mb.mb_set_process_priority = godot::api->godot_method_bind_get_method("Node", "set_process_priority");
	___mb.mb_set_process_unhandled_input = godot::api->godot_method_bind_get_method("Node", "set_process_unhandled_input");
	___mb.mb_set_process_unhandled_key_input = godot::api->godot_method_bind_get_method("Node", "set_process_unhandled_key_input");
	___mb.mb_set_scene_instance_load_placeholder = godot::api->godot_method_bind_get_method("Node", "set_scene_instance_load_placeholder");
	___mb.mb_update_configuration_warning = godot::api->godot_method_bind_get_method("Node", "update_configuration_warning");
}

Node *Node::_new()
{
	return (Node *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Node")());
}
void Node::_enter_tree() {
	___godot_icall_void(___mb.mb__enter_tree, (const Object *) this);
}

void Node::_exit_tree() {
	___godot_icall_void(___mb.mb__exit_tree, (const Object *) this);
}

String Node::_get_configuration_warning() {
	return ___godot_icall_String(___mb.mb__get_configuration_warning, (const Object *) this);
}

String Node::_get_editor_description() const {
	return ___godot_icall_String(___mb.mb__get_editor_description, (const Object *) this);
}

NodePath Node::_get_import_path() const {
	return ___godot_icall_NodePath(___mb.mb__get_import_path, (const Object *) this);
}

void Node::_input(const Ref<InputEvent> event) {
	___godot_icall_void_Object(___mb.mb__input, (const Object *) this, event.ptr());
}

void Node::_physics_process(const real_t delta) {
	___godot_icall_void_float(___mb.mb__physics_process, (const Object *) this, delta);
}

void Node::_process(const real_t delta) {
	___godot_icall_void_float(___mb.mb__process, (const Object *) this, delta);
}

void Node::_ready() {
	___godot_icall_void(___mb.mb__ready, (const Object *) this);
}

void Node::_set_editor_description(const String editor_description) {
	___godot_icall_void_String(___mb.mb__set_editor_description, (const Object *) this, editor_description);
}

void Node::_set_import_path(const NodePath import_path) {
	___godot_icall_void_NodePath(___mb.mb__set_import_path, (const Object *) this, import_path);
}

void Node::_unhandled_input(const Ref<InputEvent> event) {
	___godot_icall_void_Object(___mb.mb__unhandled_input, (const Object *) this, event.ptr());
}

void Node::_unhandled_key_input(const Ref<InputEventKey> event) {
	___godot_icall_void_Object(___mb.mb__unhandled_key_input, (const Object *) this, event.ptr());
}

void Node::add_child(const Node *node, const bool legible_unique_name) {
	___godot_icall_void_Object_bool(___mb.mb_add_child, (const Object *) this, node, legible_unique_name);
}

void Node::add_child_below_node(const Node *node, const Node *child_node, const bool legible_unique_name) {
	___godot_icall_void_Object_Object_bool(___mb.mb_add_child_below_node, (const Object *) this, node, child_node, legible_unique_name);
}

void Node::add_to_group(const String group, const bool persistent) {
	___godot_icall_void_String_bool(___mb.mb_add_to_group, (const Object *) this, group, persistent);
}

bool Node::can_process() const {
	return ___godot_icall_bool(___mb.mb_can_process, (const Object *) this);
}

Node *Node::duplicate(const int64_t flags) const {
	return (Node *) ___godot_icall_Object_int(___mb.mb_duplicate, (const Object *) this, flags);
}

Node *Node::find_node(const String mask, const bool recursive, const bool owned) const {
	return (Node *) ___godot_icall_Object_String_bool_bool(___mb.mb_find_node, (const Object *) this, mask, recursive, owned);
}

Node *Node::find_parent(const String mask) const {
	return (Node *) ___godot_icall_Object_String(___mb.mb_find_parent, (const Object *) this, mask);
}

Node *Node::get_child(const int64_t idx) const {
	return (Node *) ___godot_icall_Object_int(___mb.mb_get_child, (const Object *) this, idx);
}

int64_t Node::get_child_count() const {
	return ___godot_icall_int(___mb.mb_get_child_count, (const Object *) this);
}

Array Node::get_children() const {
	return ___godot_icall_Array(___mb.mb_get_children, (const Object *) this);
}

Ref<MultiplayerAPI> Node::get_custom_multiplayer() const {
	return Ref<MultiplayerAPI>::__internal_constructor(___godot_icall_Object(___mb.mb_get_custom_multiplayer, (const Object *) this));
}

String Node::get_filename() const {
	return ___godot_icall_String(___mb.mb_get_filename, (const Object *) this);
}

Array Node::get_groups() const {
	return ___godot_icall_Array(___mb.mb_get_groups, (const Object *) this);
}

int64_t Node::get_index() const {
	return ___godot_icall_int(___mb.mb_get_index, (const Object *) this);
}

Ref<MultiplayerAPI> Node::get_multiplayer() const {
	return Ref<MultiplayerAPI>::__internal_constructor(___godot_icall_Object(___mb.mb_get_multiplayer, (const Object *) this));
}

String Node::get_name() const {
	return ___godot_icall_String(___mb.mb_get_name, (const Object *) this);
}

int64_t Node::get_network_master() const {
	return ___godot_icall_int(___mb.mb_get_network_master, (const Object *) this);
}

Node *Node::get_node(const NodePath path) const {
	return (Node *) ___godot_icall_Object_NodePath(___mb.mb_get_node, (const Object *) this, path);
}

Array Node::get_node_and_resource(const NodePath path) {
	return ___godot_icall_Array_NodePath(___mb.mb_get_node_and_resource, (const Object *) this, path);
}

Node *Node::get_node_or_null(const NodePath path) const {
	return (Node *) ___godot_icall_Object_NodePath(___mb.mb_get_node_or_null, (const Object *) this, path);
}

Node *Node::get_owner() const {
	return (Node *) ___godot_icall_Object(___mb.mb_get_owner, (const Object *) this);
}

Node *Node::get_parent() const {
	return (Node *) ___godot_icall_Object(___mb.mb_get_parent, (const Object *) this);
}

NodePath Node::get_path() const {
	return ___godot_icall_NodePath(___mb.mb_get_path, (const Object *) this);
}

NodePath Node::get_path_to(const Node *node) const {
	return ___godot_icall_NodePath_Object(___mb.mb_get_path_to, (const Object *) this, node);
}

Node::PauseMode Node::get_pause_mode() const {
	return (Node::PauseMode) ___godot_icall_int(___mb.mb_get_pause_mode, (const Object *) this);
}

real_t Node::get_physics_process_delta_time() const {
	return ___godot_icall_float(___mb.mb_get_physics_process_delta_time, (const Object *) this);
}

int64_t Node::get_position_in_parent() const {
	return ___godot_icall_int(___mb.mb_get_position_in_parent, (const Object *) this);
}

real_t Node::get_process_delta_time() const {
	return ___godot_icall_float(___mb.mb_get_process_delta_time, (const Object *) this);
}

int64_t Node::get_process_priority() const {
	return ___godot_icall_int(___mb.mb_get_process_priority, (const Object *) this);
}

bool Node::get_scene_instance_load_placeholder() const {
	return ___godot_icall_bool(___mb.mb_get_scene_instance_load_placeholder, (const Object *) this);
}

SceneTree *Node::get_tree() const {
	return (SceneTree *) ___godot_icall_Object(___mb.mb_get_tree, (const Object *) this);
}

Viewport *Node::get_viewport() const {
	return (Viewport *) ___godot_icall_Object(___mb.mb_get_viewport, (const Object *) this);
}

bool Node::has_node(const NodePath path) const {
	return ___godot_icall_bool_NodePath(___mb.mb_has_node, (const Object *) this, path);
}

bool Node::has_node_and_resource(const NodePath path) const {
	return ___godot_icall_bool_NodePath(___mb.mb_has_node_and_resource, (const Object *) this, path);
}

bool Node::is_a_parent_of(const Node *node) const {
	return ___godot_icall_bool_Object(___mb.mb_is_a_parent_of, (const Object *) this, node);
}

bool Node::is_displayed_folded() const {
	return ___godot_icall_bool(___mb.mb_is_displayed_folded, (const Object *) this);
}

bool Node::is_greater_than(const Node *node) const {
	return ___godot_icall_bool_Object(___mb.mb_is_greater_than, (const Object *) this, node);
}

bool Node::is_in_group(const String group) const {
	return ___godot_icall_bool_String(___mb.mb_is_in_group, (const Object *) this, group);
}

bool Node::is_inside_tree() const {
	return ___godot_icall_bool(___mb.mb_is_inside_tree, (const Object *) this);
}

bool Node::is_network_master() const {
	return ___godot_icall_bool(___mb.mb_is_network_master, (const Object *) this);
}

bool Node::is_physics_processing() const {
	return ___godot_icall_bool(___mb.mb_is_physics_processing, (const Object *) this);
}

bool Node::is_physics_processing_internal() const {
	return ___godot_icall_bool(___mb.mb_is_physics_processing_internal, (const Object *) this);
}

bool Node::is_processing() const {
	return ___godot_icall_bool(___mb.mb_is_processing, (const Object *) this);
}

bool Node::is_processing_input() const {
	return ___godot_icall_bool(___mb.mb_is_processing_input, (const Object *) this);
}

bool Node::is_processing_internal() const {
	return ___godot_icall_bool(___mb.mb_is_processing_internal, (const Object *) this);
}

bool Node::is_processing_unhandled_input() const {
	return ___godot_icall_bool(___mb.mb_is_processing_unhandled_input, (const Object *) this);
}

bool Node::is_processing_unhandled_key_input() const {
	return ___godot_icall_bool(___mb.mb_is_processing_unhandled_key_input, (const Object *) this);
}

void Node::move_child(const Node *child_node, const int64_t to_position) {
	___godot_icall_void_Object_int(___mb.mb_move_child, (const Object *) this, child_node, to_position);
}

void Node::print_stray_nodes() {
	___godot_icall_void(___mb.mb_print_stray_nodes, (const Object *) this);
}

void Node::print_tree() {
	___godot_icall_void(___mb.mb_print_tree, (const Object *) this);
}

void Node::print_tree_pretty() {
	___godot_icall_void(___mb.mb_print_tree_pretty, (const Object *) this);
}

void Node::propagate_call(const String method, const Array args, const bool parent_first) {
	___godot_icall_void_String_Array_bool(___mb.mb_propagate_call, (const Object *) this, method, args, parent_first);
}

void Node::propagate_notification(const int64_t what) {
	___godot_icall_void_int(___mb.mb_propagate_notification, (const Object *) this, what);
}

void Node::queue_free() {
	___godot_icall_void(___mb.mb_queue_free, (const Object *) this);
}

void Node::raise() {
	___godot_icall_void(___mb.mb_raise, (const Object *) this);
}

void Node::remove_and_skip() {
	___godot_icall_void(___mb.mb_remove_and_skip, (const Object *) this);
}

void Node::remove_child(const Node *node) {
	___godot_icall_void_Object(___mb.mb_remove_child, (const Object *) this, node);
}

void Node::remove_from_group(const String group) {
	___godot_icall_void_String(___mb.mb_remove_from_group, (const Object *) this, group);
}

void Node::replace_by(const Node *node, const bool keep_data) {
	___godot_icall_void_Object_bool(___mb.mb_replace_by, (const Object *) this, node, keep_data);
}

void Node::request_ready() {
	___godot_icall_void(___mb.mb_request_ready, (const Object *) this);
}

Variant Node::rpc(const String method, const Array& __var_args) {
	Variant __given_args[1];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);

	__given_args[0] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 1));

	__args[0] = (godot_variant *) &__given_args[0];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 1] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_rpc, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 1), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);

	return __result;
}

void Node::rpc_config(const String method, const int64_t mode) {
	___godot_icall_void_String_int(___mb.mb_rpc_config, (const Object *) this, method, mode);
}

Variant Node::rpc_id(const int64_t peer_id, const String method, const Array& __var_args) {
	Variant __given_args[2];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[1]);

	__given_args[0] = peer_id;
	__given_args[1] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 2));

	__args[0] = (godot_variant *) &__given_args[0];
	__args[1] = (godot_variant *) &__given_args[1];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 2] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_rpc_id, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 2), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_destroy((godot_variant *) &__given_args[1]);

	return __result;
}

Variant Node::rpc_unreliable(const String method, const Array& __var_args) {
	Variant __given_args[1];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);

	__given_args[0] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 1));

	__args[0] = (godot_variant *) &__given_args[0];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 1] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_rpc_unreliable, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 1), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);

	return __result;
}

Variant Node::rpc_unreliable_id(const int64_t peer_id, const String method, const Array& __var_args) {
	Variant __given_args[2];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[1]);

	__given_args[0] = peer_id;
	__given_args[1] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 2));

	__args[0] = (godot_variant *) &__given_args[0];
	__args[1] = (godot_variant *) &__given_args[1];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 2] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_rpc_unreliable_id, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 2), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_destroy((godot_variant *) &__given_args[1]);

	return __result;
}

void Node::rset(const String property, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_rset, (const Object *) this, property, value);
}

void Node::rset_config(const String property, const int64_t mode) {
	___godot_icall_void_String_int(___mb.mb_rset_config, (const Object *) this, property, mode);
}

void Node::rset_id(const int64_t peer_id, const String property, const Variant value) {
	___godot_icall_void_int_String_Variant(___mb.mb_rset_id, (const Object *) this, peer_id, property, value);
}

void Node::rset_unreliable(const String property, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_rset_unreliable, (const Object *) this, property, value);
}

void Node::rset_unreliable_id(const int64_t peer_id, const String property, const Variant value) {
	___godot_icall_void_int_String_Variant(___mb.mb_rset_unreliable_id, (const Object *) this, peer_id, property, value);
}

void Node::set_custom_multiplayer(const Ref<MultiplayerAPI> api) {
	___godot_icall_void_Object(___mb.mb_set_custom_multiplayer, (const Object *) this, api.ptr());
}

void Node::set_display_folded(const bool fold) {
	___godot_icall_void_bool(___mb.mb_set_display_folded, (const Object *) this, fold);
}

void Node::set_filename(const String filename) {
	___godot_icall_void_String(___mb.mb_set_filename, (const Object *) this, filename);
}

void Node::set_name(const String name) {
	___godot_icall_void_String(___mb.mb_set_name, (const Object *) this, name);
}

void Node::set_network_master(const int64_t id, const bool recursive) {
	___godot_icall_void_int_bool(___mb.mb_set_network_master, (const Object *) this, id, recursive);
}

void Node::set_owner(const Node *owner) {
	___godot_icall_void_Object(___mb.mb_set_owner, (const Object *) this, owner);
}

void Node::set_pause_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_pause_mode, (const Object *) this, mode);
}

void Node::set_physics_process(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_physics_process, (const Object *) this, enable);
}

void Node::set_physics_process_internal(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_physics_process_internal, (const Object *) this, enable);
}

void Node::set_process(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_process, (const Object *) this, enable);
}

void Node::set_process_input(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_process_input, (const Object *) this, enable);
}

void Node::set_process_internal(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_process_internal, (const Object *) this, enable);
}

void Node::set_process_priority(const int64_t priority) {
	___godot_icall_void_int(___mb.mb_set_process_priority, (const Object *) this, priority);
}

void Node::set_process_unhandled_input(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_process_unhandled_input, (const Object *) this, enable);
}

void Node::set_process_unhandled_key_input(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_process_unhandled_key_input, (const Object *) this, enable);
}

void Node::set_scene_instance_load_placeholder(const bool load_placeholder) {
	___godot_icall_void_bool(___mb.mb_set_scene_instance_load_placeholder, (const Object *) this, load_placeholder);
}

void Node::update_configuration_warning() {
	___godot_icall_void(___mb.mb_update_configuration_warning, (const Object *) this);
}

}