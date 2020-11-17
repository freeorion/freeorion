#include "MultiplayerAPI.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "NetworkedMultiplayerPeer.hpp"
#include "Node.hpp"


namespace godot {


MultiplayerAPI::___method_bindings MultiplayerAPI::___mb = {};

void MultiplayerAPI::___init_method_bindings() {
	___mb.mb__add_peer = godot::api->godot_method_bind_get_method("MultiplayerAPI", "_add_peer");
	___mb.mb__connected_to_server = godot::api->godot_method_bind_get_method("MultiplayerAPI", "_connected_to_server");
	___mb.mb__connection_failed = godot::api->godot_method_bind_get_method("MultiplayerAPI", "_connection_failed");
	___mb.mb__del_peer = godot::api->godot_method_bind_get_method("MultiplayerAPI", "_del_peer");
	___mb.mb__server_disconnected = godot::api->godot_method_bind_get_method("MultiplayerAPI", "_server_disconnected");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("MultiplayerAPI", "clear");
	___mb.mb_get_network_connected_peers = godot::api->godot_method_bind_get_method("MultiplayerAPI", "get_network_connected_peers");
	___mb.mb_get_network_peer = godot::api->godot_method_bind_get_method("MultiplayerAPI", "get_network_peer");
	___mb.mb_get_network_unique_id = godot::api->godot_method_bind_get_method("MultiplayerAPI", "get_network_unique_id");
	___mb.mb_get_rpc_sender_id = godot::api->godot_method_bind_get_method("MultiplayerAPI", "get_rpc_sender_id");
	___mb.mb_has_network_peer = godot::api->godot_method_bind_get_method("MultiplayerAPI", "has_network_peer");
	___mb.mb_is_network_server = godot::api->godot_method_bind_get_method("MultiplayerAPI", "is_network_server");
	___mb.mb_is_object_decoding_allowed = godot::api->godot_method_bind_get_method("MultiplayerAPI", "is_object_decoding_allowed");
	___mb.mb_is_refusing_new_network_connections = godot::api->godot_method_bind_get_method("MultiplayerAPI", "is_refusing_new_network_connections");
	___mb.mb_poll = godot::api->godot_method_bind_get_method("MultiplayerAPI", "poll");
	___mb.mb_send_bytes = godot::api->godot_method_bind_get_method("MultiplayerAPI", "send_bytes");
	___mb.mb_set_allow_object_decoding = godot::api->godot_method_bind_get_method("MultiplayerAPI", "set_allow_object_decoding");
	___mb.mb_set_network_peer = godot::api->godot_method_bind_get_method("MultiplayerAPI", "set_network_peer");
	___mb.mb_set_refuse_new_network_connections = godot::api->godot_method_bind_get_method("MultiplayerAPI", "set_refuse_new_network_connections");
	___mb.mb_set_root_node = godot::api->godot_method_bind_get_method("MultiplayerAPI", "set_root_node");
}

MultiplayerAPI *MultiplayerAPI::_new()
{
	return (MultiplayerAPI *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MultiplayerAPI")());
}
void MultiplayerAPI::_add_peer(const int64_t id) {
	___godot_icall_void_int(___mb.mb__add_peer, (const Object *) this, id);
}

void MultiplayerAPI::_connected_to_server() {
	___godot_icall_void(___mb.mb__connected_to_server, (const Object *) this);
}

void MultiplayerAPI::_connection_failed() {
	___godot_icall_void(___mb.mb__connection_failed, (const Object *) this);
}

void MultiplayerAPI::_del_peer(const int64_t id) {
	___godot_icall_void_int(___mb.mb__del_peer, (const Object *) this, id);
}

void MultiplayerAPI::_server_disconnected() {
	___godot_icall_void(___mb.mb__server_disconnected, (const Object *) this);
}

void MultiplayerAPI::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

PoolIntArray MultiplayerAPI::get_network_connected_peers() const {
	return ___godot_icall_PoolIntArray(___mb.mb_get_network_connected_peers, (const Object *) this);
}

Ref<NetworkedMultiplayerPeer> MultiplayerAPI::get_network_peer() const {
	return Ref<NetworkedMultiplayerPeer>::__internal_constructor(___godot_icall_Object(___mb.mb_get_network_peer, (const Object *) this));
}

int64_t MultiplayerAPI::get_network_unique_id() const {
	return ___godot_icall_int(___mb.mb_get_network_unique_id, (const Object *) this);
}

int64_t MultiplayerAPI::get_rpc_sender_id() const {
	return ___godot_icall_int(___mb.mb_get_rpc_sender_id, (const Object *) this);
}

bool MultiplayerAPI::has_network_peer() const {
	return ___godot_icall_bool(___mb.mb_has_network_peer, (const Object *) this);
}

bool MultiplayerAPI::is_network_server() const {
	return ___godot_icall_bool(___mb.mb_is_network_server, (const Object *) this);
}

bool MultiplayerAPI::is_object_decoding_allowed() const {
	return ___godot_icall_bool(___mb.mb_is_object_decoding_allowed, (const Object *) this);
}

bool MultiplayerAPI::is_refusing_new_network_connections() const {
	return ___godot_icall_bool(___mb.mb_is_refusing_new_network_connections, (const Object *) this);
}

void MultiplayerAPI::poll() {
	___godot_icall_void(___mb.mb_poll, (const Object *) this);
}

Error MultiplayerAPI::send_bytes(const PoolByteArray bytes, const int64_t id, const int64_t mode) {
	return (Error) ___godot_icall_int_PoolByteArray_int_int(___mb.mb_send_bytes, (const Object *) this, bytes, id, mode);
}

void MultiplayerAPI::set_allow_object_decoding(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_allow_object_decoding, (const Object *) this, enable);
}

void MultiplayerAPI::set_network_peer(const Ref<NetworkedMultiplayerPeer> peer) {
	___godot_icall_void_Object(___mb.mb_set_network_peer, (const Object *) this, peer.ptr());
}

void MultiplayerAPI::set_refuse_new_network_connections(const bool refuse) {
	___godot_icall_void_bool(___mb.mb_set_refuse_new_network_connections, (const Object *) this, refuse);
}

void MultiplayerAPI::set_root_node(const Node *node) {
	___godot_icall_void_Object(___mb.mb_set_root_node, (const Object *) this, node);
}

}