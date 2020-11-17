#include "SceneTree.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "PackedScene.hpp"
#include "SceneTreeTimer.hpp"
#include "MultiplayerAPI.hpp"
#include "NetworkedMultiplayerPeer.hpp"
#include "Viewport.hpp"
#include "Object.hpp"


namespace godot {


SceneTree::___method_bindings SceneTree::___mb = {};

void SceneTree::___init_method_bindings() {
	___mb.mb__change_scene = godot::api->godot_method_bind_get_method("SceneTree", "_change_scene");
	___mb.mb__connected_to_server = godot::api->godot_method_bind_get_method("SceneTree", "_connected_to_server");
	___mb.mb__connection_failed = godot::api->godot_method_bind_get_method("SceneTree", "_connection_failed");
	___mb.mb__network_peer_connected = godot::api->godot_method_bind_get_method("SceneTree", "_network_peer_connected");
	___mb.mb__network_peer_disconnected = godot::api->godot_method_bind_get_method("SceneTree", "_network_peer_disconnected");
	___mb.mb__server_disconnected = godot::api->godot_method_bind_get_method("SceneTree", "_server_disconnected");
	___mb.mb_call_group = godot::api->godot_method_bind_get_method("SceneTree", "call_group");
	___mb.mb_call_group_flags = godot::api->godot_method_bind_get_method("SceneTree", "call_group_flags");
	___mb.mb_change_scene = godot::api->godot_method_bind_get_method("SceneTree", "change_scene");
	___mb.mb_change_scene_to = godot::api->godot_method_bind_get_method("SceneTree", "change_scene_to");
	___mb.mb_create_timer = godot::api->godot_method_bind_get_method("SceneTree", "create_timer");
	___mb.mb_get_current_scene = godot::api->godot_method_bind_get_method("SceneTree", "get_current_scene");
	___mb.mb_get_edited_scene_root = godot::api->godot_method_bind_get_method("SceneTree", "get_edited_scene_root");
	___mb.mb_get_frame = godot::api->godot_method_bind_get_method("SceneTree", "get_frame");
	___mb.mb_get_multiplayer = godot::api->godot_method_bind_get_method("SceneTree", "get_multiplayer");
	___mb.mb_get_network_connected_peers = godot::api->godot_method_bind_get_method("SceneTree", "get_network_connected_peers");
	___mb.mb_get_network_peer = godot::api->godot_method_bind_get_method("SceneTree", "get_network_peer");
	___mb.mb_get_network_unique_id = godot::api->godot_method_bind_get_method("SceneTree", "get_network_unique_id");
	___mb.mb_get_node_count = godot::api->godot_method_bind_get_method("SceneTree", "get_node_count");
	___mb.mb_get_nodes_in_group = godot::api->godot_method_bind_get_method("SceneTree", "get_nodes_in_group");
	___mb.mb_get_root = godot::api->godot_method_bind_get_method("SceneTree", "get_root");
	___mb.mb_get_rpc_sender_id = godot::api->godot_method_bind_get_method("SceneTree", "get_rpc_sender_id");
	___mb.mb_has_group = godot::api->godot_method_bind_get_method("SceneTree", "has_group");
	___mb.mb_has_network_peer = godot::api->godot_method_bind_get_method("SceneTree", "has_network_peer");
	___mb.mb_is_debugging_collisions_hint = godot::api->godot_method_bind_get_method("SceneTree", "is_debugging_collisions_hint");
	___mb.mb_is_debugging_navigation_hint = godot::api->godot_method_bind_get_method("SceneTree", "is_debugging_navigation_hint");
	___mb.mb_is_input_handled = godot::api->godot_method_bind_get_method("SceneTree", "is_input_handled");
	___mb.mb_is_multiplayer_poll_enabled = godot::api->godot_method_bind_get_method("SceneTree", "is_multiplayer_poll_enabled");
	___mb.mb_is_network_server = godot::api->godot_method_bind_get_method("SceneTree", "is_network_server");
	___mb.mb_is_paused = godot::api->godot_method_bind_get_method("SceneTree", "is_paused");
	___mb.mb_is_refusing_new_network_connections = godot::api->godot_method_bind_get_method("SceneTree", "is_refusing_new_network_connections");
	___mb.mb_is_using_font_oversampling = godot::api->godot_method_bind_get_method("SceneTree", "is_using_font_oversampling");
	___mb.mb_notify_group = godot::api->godot_method_bind_get_method("SceneTree", "notify_group");
	___mb.mb_notify_group_flags = godot::api->godot_method_bind_get_method("SceneTree", "notify_group_flags");
	___mb.mb_queue_delete = godot::api->godot_method_bind_get_method("SceneTree", "queue_delete");
	___mb.mb_quit = godot::api->godot_method_bind_get_method("SceneTree", "quit");
	___mb.mb_reload_current_scene = godot::api->godot_method_bind_get_method("SceneTree", "reload_current_scene");
	___mb.mb_set_auto_accept_quit = godot::api->godot_method_bind_get_method("SceneTree", "set_auto_accept_quit");
	___mb.mb_set_current_scene = godot::api->godot_method_bind_get_method("SceneTree", "set_current_scene");
	___mb.mb_set_debug_collisions_hint = godot::api->godot_method_bind_get_method("SceneTree", "set_debug_collisions_hint");
	___mb.mb_set_debug_navigation_hint = godot::api->godot_method_bind_get_method("SceneTree", "set_debug_navigation_hint");
	___mb.mb_set_edited_scene_root = godot::api->godot_method_bind_get_method("SceneTree", "set_edited_scene_root");
	___mb.mb_set_group = godot::api->godot_method_bind_get_method("SceneTree", "set_group");
	___mb.mb_set_group_flags = godot::api->godot_method_bind_get_method("SceneTree", "set_group_flags");
	___mb.mb_set_input_as_handled = godot::api->godot_method_bind_get_method("SceneTree", "set_input_as_handled");
	___mb.mb_set_multiplayer = godot::api->godot_method_bind_get_method("SceneTree", "set_multiplayer");
	___mb.mb_set_multiplayer_poll_enabled = godot::api->godot_method_bind_get_method("SceneTree", "set_multiplayer_poll_enabled");
	___mb.mb_set_network_peer = godot::api->godot_method_bind_get_method("SceneTree", "set_network_peer");
	___mb.mb_set_pause = godot::api->godot_method_bind_get_method("SceneTree", "set_pause");
	___mb.mb_set_quit_on_go_back = godot::api->godot_method_bind_get_method("SceneTree", "set_quit_on_go_back");
	___mb.mb_set_refuse_new_network_connections = godot::api->godot_method_bind_get_method("SceneTree", "set_refuse_new_network_connections");
	___mb.mb_set_screen_stretch = godot::api->godot_method_bind_get_method("SceneTree", "set_screen_stretch");
	___mb.mb_set_use_font_oversampling = godot::api->godot_method_bind_get_method("SceneTree", "set_use_font_oversampling");
}

SceneTree *SceneTree::_new()
{
	return (SceneTree *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SceneTree")());
}
void SceneTree::_change_scene(const Node *arg0) {
	___godot_icall_void_Object(___mb.mb__change_scene, (const Object *) this, arg0);
}

void SceneTree::_connected_to_server() {
	___godot_icall_void(___mb.mb__connected_to_server, (const Object *) this);
}

void SceneTree::_connection_failed() {
	___godot_icall_void(___mb.mb__connection_failed, (const Object *) this);
}

void SceneTree::_network_peer_connected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__network_peer_connected, (const Object *) this, arg0);
}

void SceneTree::_network_peer_disconnected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__network_peer_disconnected, (const Object *) this, arg0);
}

void SceneTree::_server_disconnected() {
	___godot_icall_void(___mb.mb__server_disconnected, (const Object *) this);
}

Variant SceneTree::call_group(const String group, const String method, const Array& __var_args) {
	Variant __given_args[2];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[1]);

	__given_args[0] = group;
	__given_args[1] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 2));

	__args[0] = (godot_variant *) &__given_args[0];
	__args[1] = (godot_variant *) &__given_args[1];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 2] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_call_group, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 2), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_destroy((godot_variant *) &__given_args[1]);

	return __result;
}

Variant SceneTree::call_group_flags(const int64_t flags, const String group, const String method, const Array& __var_args) {
	Variant __given_args[3];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[1]);
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[2]);

	__given_args[0] = flags;
	__given_args[1] = group;
	__given_args[2] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 3));

	__args[0] = (godot_variant *) &__given_args[0];
	__args[1] = (godot_variant *) &__given_args[1];
	__args[2] = (godot_variant *) &__given_args[2];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 3] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_call_group_flags, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 3), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);
	godot::api->godot_variant_destroy((godot_variant *) &__given_args[1]);
	godot::api->godot_variant_destroy((godot_variant *) &__given_args[2]);

	return __result;
}

Error SceneTree::change_scene(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_change_scene, (const Object *) this, path);
}

Error SceneTree::change_scene_to(const Ref<PackedScene> packed_scene) {
	return (Error) ___godot_icall_int_Object(___mb.mb_change_scene_to, (const Object *) this, packed_scene.ptr());
}

Ref<SceneTreeTimer> SceneTree::create_timer(const real_t time_sec, const bool pause_mode_process) {
	return Ref<SceneTreeTimer>::__internal_constructor(___godot_icall_Object_float_bool(___mb.mb_create_timer, (const Object *) this, time_sec, pause_mode_process));
}

Node *SceneTree::get_current_scene() const {
	return (Node *) ___godot_icall_Object(___mb.mb_get_current_scene, (const Object *) this);
}

Node *SceneTree::get_edited_scene_root() const {
	return (Node *) ___godot_icall_Object(___mb.mb_get_edited_scene_root, (const Object *) this);
}

int64_t SceneTree::get_frame() const {
	return ___godot_icall_int(___mb.mb_get_frame, (const Object *) this);
}

Ref<MultiplayerAPI> SceneTree::get_multiplayer() const {
	return Ref<MultiplayerAPI>::__internal_constructor(___godot_icall_Object(___mb.mb_get_multiplayer, (const Object *) this));
}

PoolIntArray SceneTree::get_network_connected_peers() const {
	return ___godot_icall_PoolIntArray(___mb.mb_get_network_connected_peers, (const Object *) this);
}

Ref<NetworkedMultiplayerPeer> SceneTree::get_network_peer() const {
	return Ref<NetworkedMultiplayerPeer>::__internal_constructor(___godot_icall_Object(___mb.mb_get_network_peer, (const Object *) this));
}

int64_t SceneTree::get_network_unique_id() const {
	return ___godot_icall_int(___mb.mb_get_network_unique_id, (const Object *) this);
}

int64_t SceneTree::get_node_count() const {
	return ___godot_icall_int(___mb.mb_get_node_count, (const Object *) this);
}

Array SceneTree::get_nodes_in_group(const String group) {
	return ___godot_icall_Array_String(___mb.mb_get_nodes_in_group, (const Object *) this, group);
}

Viewport *SceneTree::get_root() const {
	return (Viewport *) ___godot_icall_Object(___mb.mb_get_root, (const Object *) this);
}

int64_t SceneTree::get_rpc_sender_id() const {
	return ___godot_icall_int(___mb.mb_get_rpc_sender_id, (const Object *) this);
}

bool SceneTree::has_group(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_group, (const Object *) this, name);
}

bool SceneTree::has_network_peer() const {
	return ___godot_icall_bool(___mb.mb_has_network_peer, (const Object *) this);
}

bool SceneTree::is_debugging_collisions_hint() const {
	return ___godot_icall_bool(___mb.mb_is_debugging_collisions_hint, (const Object *) this);
}

bool SceneTree::is_debugging_navigation_hint() const {
	return ___godot_icall_bool(___mb.mb_is_debugging_navigation_hint, (const Object *) this);
}

bool SceneTree::is_input_handled() {
	return ___godot_icall_bool(___mb.mb_is_input_handled, (const Object *) this);
}

bool SceneTree::is_multiplayer_poll_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_multiplayer_poll_enabled, (const Object *) this);
}

bool SceneTree::is_network_server() const {
	return ___godot_icall_bool(___mb.mb_is_network_server, (const Object *) this);
}

bool SceneTree::is_paused() const {
	return ___godot_icall_bool(___mb.mb_is_paused, (const Object *) this);
}

bool SceneTree::is_refusing_new_network_connections() const {
	return ___godot_icall_bool(___mb.mb_is_refusing_new_network_connections, (const Object *) this);
}

bool SceneTree::is_using_font_oversampling() const {
	return ___godot_icall_bool(___mb.mb_is_using_font_oversampling, (const Object *) this);
}

void SceneTree::notify_group(const String group, const int64_t notification) {
	___godot_icall_void_String_int(___mb.mb_notify_group, (const Object *) this, group, notification);
}

void SceneTree::notify_group_flags(const int64_t call_flags, const String group, const int64_t notification) {
	___godot_icall_void_int_String_int(___mb.mb_notify_group_flags, (const Object *) this, call_flags, group, notification);
}

void SceneTree::queue_delete(const Object *obj) {
	___godot_icall_void_Object(___mb.mb_queue_delete, (const Object *) this, obj);
}

void SceneTree::quit(const int64_t exit_code) {
	___godot_icall_void_int(___mb.mb_quit, (const Object *) this, exit_code);
}

Error SceneTree::reload_current_scene() {
	return (Error) ___godot_icall_int(___mb.mb_reload_current_scene, (const Object *) this);
}

void SceneTree::set_auto_accept_quit(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_auto_accept_quit, (const Object *) this, enabled);
}

void SceneTree::set_current_scene(const Node *child_node) {
	___godot_icall_void_Object(___mb.mb_set_current_scene, (const Object *) this, child_node);
}

void SceneTree::set_debug_collisions_hint(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_debug_collisions_hint, (const Object *) this, enable);
}

void SceneTree::set_debug_navigation_hint(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_debug_navigation_hint, (const Object *) this, enable);
}

void SceneTree::set_edited_scene_root(const Node *scene) {
	___godot_icall_void_Object(___mb.mb_set_edited_scene_root, (const Object *) this, scene);
}

void SceneTree::set_group(const String group, const String property, const Variant value) {
	___godot_icall_void_String_String_Variant(___mb.mb_set_group, (const Object *) this, group, property, value);
}

void SceneTree::set_group_flags(const int64_t call_flags, const String group, const String property, const Variant value) {
	___godot_icall_void_int_String_String_Variant(___mb.mb_set_group_flags, (const Object *) this, call_flags, group, property, value);
}

void SceneTree::set_input_as_handled() {
	___godot_icall_void(___mb.mb_set_input_as_handled, (const Object *) this);
}

void SceneTree::set_multiplayer(const Ref<MultiplayerAPI> multiplayer) {
	___godot_icall_void_Object(___mb.mb_set_multiplayer, (const Object *) this, multiplayer.ptr());
}

void SceneTree::set_multiplayer_poll_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_multiplayer_poll_enabled, (const Object *) this, enabled);
}

void SceneTree::set_network_peer(const Ref<NetworkedMultiplayerPeer> peer) {
	___godot_icall_void_Object(___mb.mb_set_network_peer, (const Object *) this, peer.ptr());
}

void SceneTree::set_pause(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_pause, (const Object *) this, enable);
}

void SceneTree::set_quit_on_go_back(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_quit_on_go_back, (const Object *) this, enabled);
}

void SceneTree::set_refuse_new_network_connections(const bool refuse) {
	___godot_icall_void_bool(___mb.mb_set_refuse_new_network_connections, (const Object *) this, refuse);
}

void SceneTree::set_screen_stretch(const int64_t mode, const int64_t aspect, const Vector2 minsize, const real_t shrink) {
	___godot_icall_void_int_int_Vector2_float(___mb.mb_set_screen_stretch, (const Object *) this, mode, aspect, minsize, shrink);
}

void SceneTree::set_use_font_oversampling(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_font_oversampling, (const Object *) this, enable);
}

}