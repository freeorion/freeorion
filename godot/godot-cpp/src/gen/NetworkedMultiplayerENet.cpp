#include "NetworkedMultiplayerENet.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


NetworkedMultiplayerENet::___method_bindings NetworkedMultiplayerENet::___mb = {};

void NetworkedMultiplayerENet::___init_method_bindings() {
	___mb.mb_close_connection = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "close_connection");
	___mb.mb_create_client = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "create_client");
	___mb.mb_create_server = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "create_server");
	___mb.mb_disconnect_peer = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "disconnect_peer");
	___mb.mb_get_channel_count = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "get_channel_count");
	___mb.mb_get_compression_mode = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "get_compression_mode");
	___mb.mb_get_last_packet_channel = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "get_last_packet_channel");
	___mb.mb_get_packet_channel = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "get_packet_channel");
	___mb.mb_get_peer_address = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "get_peer_address");
	___mb.mb_get_peer_port = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "get_peer_port");
	___mb.mb_get_transfer_channel = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "get_transfer_channel");
	___mb.mb_is_always_ordered = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "is_always_ordered");
	___mb.mb_is_server_relay_enabled = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "is_server_relay_enabled");
	___mb.mb_set_always_ordered = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "set_always_ordered");
	___mb.mb_set_bind_ip = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "set_bind_ip");
	___mb.mb_set_channel_count = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "set_channel_count");
	___mb.mb_set_compression_mode = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "set_compression_mode");
	___mb.mb_set_server_relay_enabled = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "set_server_relay_enabled");
	___mb.mb_set_transfer_channel = godot::api->godot_method_bind_get_method("NetworkedMultiplayerENet", "set_transfer_channel");
}

NetworkedMultiplayerENet *NetworkedMultiplayerENet::_new()
{
	return (NetworkedMultiplayerENet *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"NetworkedMultiplayerENet")());
}
void NetworkedMultiplayerENet::close_connection(const int64_t wait_usec) {
	___godot_icall_void_int(___mb.mb_close_connection, (const Object *) this, wait_usec);
}

Error NetworkedMultiplayerENet::create_client(const String address, const int64_t port, const int64_t in_bandwidth, const int64_t out_bandwidth, const int64_t client_port) {
	return (Error) ___godot_icall_int_String_int_int_int_int(___mb.mb_create_client, (const Object *) this, address, port, in_bandwidth, out_bandwidth, client_port);
}

Error NetworkedMultiplayerENet::create_server(const int64_t port, const int64_t max_clients, const int64_t in_bandwidth, const int64_t out_bandwidth) {
	return (Error) ___godot_icall_int_int_int_int_int(___mb.mb_create_server, (const Object *) this, port, max_clients, in_bandwidth, out_bandwidth);
}

void NetworkedMultiplayerENet::disconnect_peer(const int64_t id, const bool now) {
	___godot_icall_void_int_bool(___mb.mb_disconnect_peer, (const Object *) this, id, now);
}

int64_t NetworkedMultiplayerENet::get_channel_count() const {
	return ___godot_icall_int(___mb.mb_get_channel_count, (const Object *) this);
}

NetworkedMultiplayerENet::CompressionMode NetworkedMultiplayerENet::get_compression_mode() const {
	return (NetworkedMultiplayerENet::CompressionMode) ___godot_icall_int(___mb.mb_get_compression_mode, (const Object *) this);
}

int64_t NetworkedMultiplayerENet::get_last_packet_channel() const {
	return ___godot_icall_int(___mb.mb_get_last_packet_channel, (const Object *) this);
}

int64_t NetworkedMultiplayerENet::get_packet_channel() const {
	return ___godot_icall_int(___mb.mb_get_packet_channel, (const Object *) this);
}

String NetworkedMultiplayerENet::get_peer_address(const int64_t id) const {
	return ___godot_icall_String_int(___mb.mb_get_peer_address, (const Object *) this, id);
}

int64_t NetworkedMultiplayerENet::get_peer_port(const int64_t id) const {
	return ___godot_icall_int_int(___mb.mb_get_peer_port, (const Object *) this, id);
}

int64_t NetworkedMultiplayerENet::get_transfer_channel() const {
	return ___godot_icall_int(___mb.mb_get_transfer_channel, (const Object *) this);
}

bool NetworkedMultiplayerENet::is_always_ordered() const {
	return ___godot_icall_bool(___mb.mb_is_always_ordered, (const Object *) this);
}

bool NetworkedMultiplayerENet::is_server_relay_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_server_relay_enabled, (const Object *) this);
}

void NetworkedMultiplayerENet::set_always_ordered(const bool ordered) {
	___godot_icall_void_bool(___mb.mb_set_always_ordered, (const Object *) this, ordered);
}

void NetworkedMultiplayerENet::set_bind_ip(const String ip) {
	___godot_icall_void_String(___mb.mb_set_bind_ip, (const Object *) this, ip);
}

void NetworkedMultiplayerENet::set_channel_count(const int64_t channels) {
	___godot_icall_void_int(___mb.mb_set_channel_count, (const Object *) this, channels);
}

void NetworkedMultiplayerENet::set_compression_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_compression_mode, (const Object *) this, mode);
}

void NetworkedMultiplayerENet::set_server_relay_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_server_relay_enabled, (const Object *) this, enabled);
}

void NetworkedMultiplayerENet::set_transfer_channel(const int64_t channel) {
	___godot_icall_void_int(___mb.mb_set_transfer_channel, (const Object *) this, channel);
}

}