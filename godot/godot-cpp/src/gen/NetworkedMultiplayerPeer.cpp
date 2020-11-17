#include "NetworkedMultiplayerPeer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


NetworkedMultiplayerPeer::___method_bindings NetworkedMultiplayerPeer::___mb = {};

void NetworkedMultiplayerPeer::___init_method_bindings() {
	___mb.mb_get_connection_status = godot::api->godot_method_bind_get_method("NetworkedMultiplayerPeer", "get_connection_status");
	___mb.mb_get_packet_peer = godot::api->godot_method_bind_get_method("NetworkedMultiplayerPeer", "get_packet_peer");
	___mb.mb_get_transfer_mode = godot::api->godot_method_bind_get_method("NetworkedMultiplayerPeer", "get_transfer_mode");
	___mb.mb_get_unique_id = godot::api->godot_method_bind_get_method("NetworkedMultiplayerPeer", "get_unique_id");
	___mb.mb_is_refusing_new_connections = godot::api->godot_method_bind_get_method("NetworkedMultiplayerPeer", "is_refusing_new_connections");
	___mb.mb_poll = godot::api->godot_method_bind_get_method("NetworkedMultiplayerPeer", "poll");
	___mb.mb_set_refuse_new_connections = godot::api->godot_method_bind_get_method("NetworkedMultiplayerPeer", "set_refuse_new_connections");
	___mb.mb_set_target_peer = godot::api->godot_method_bind_get_method("NetworkedMultiplayerPeer", "set_target_peer");
	___mb.mb_set_transfer_mode = godot::api->godot_method_bind_get_method("NetworkedMultiplayerPeer", "set_transfer_mode");
}

NetworkedMultiplayerPeer::ConnectionStatus NetworkedMultiplayerPeer::get_connection_status() const {
	return (NetworkedMultiplayerPeer::ConnectionStatus) ___godot_icall_int(___mb.mb_get_connection_status, (const Object *) this);
}

int64_t NetworkedMultiplayerPeer::get_packet_peer() const {
	return ___godot_icall_int(___mb.mb_get_packet_peer, (const Object *) this);
}

NetworkedMultiplayerPeer::TransferMode NetworkedMultiplayerPeer::get_transfer_mode() const {
	return (NetworkedMultiplayerPeer::TransferMode) ___godot_icall_int(___mb.mb_get_transfer_mode, (const Object *) this);
}

int64_t NetworkedMultiplayerPeer::get_unique_id() const {
	return ___godot_icall_int(___mb.mb_get_unique_id, (const Object *) this);
}

bool NetworkedMultiplayerPeer::is_refusing_new_connections() const {
	return ___godot_icall_bool(___mb.mb_is_refusing_new_connections, (const Object *) this);
}

void NetworkedMultiplayerPeer::poll() {
	___godot_icall_void(___mb.mb_poll, (const Object *) this);
}

void NetworkedMultiplayerPeer::set_refuse_new_connections(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_refuse_new_connections, (const Object *) this, enable);
}

void NetworkedMultiplayerPeer::set_target_peer(const int64_t id) {
	___godot_icall_void_int(___mb.mb_set_target_peer, (const Object *) this, id);
}

void NetworkedMultiplayerPeer::set_transfer_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_transfer_mode, (const Object *) this, mode);
}

}