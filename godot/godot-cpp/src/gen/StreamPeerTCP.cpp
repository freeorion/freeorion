#include "StreamPeerTCP.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


StreamPeerTCP::___method_bindings StreamPeerTCP::___mb = {};

void StreamPeerTCP::___init_method_bindings() {
	___mb.mb_connect_to_host = godot::api->godot_method_bind_get_method("StreamPeerTCP", "connect_to_host");
	___mb.mb_disconnect_from_host = godot::api->godot_method_bind_get_method("StreamPeerTCP", "disconnect_from_host");
	___mb.mb_get_connected_host = godot::api->godot_method_bind_get_method("StreamPeerTCP", "get_connected_host");
	___mb.mb_get_connected_port = godot::api->godot_method_bind_get_method("StreamPeerTCP", "get_connected_port");
	___mb.mb_get_status = godot::api->godot_method_bind_get_method("StreamPeerTCP", "get_status");
	___mb.mb_is_connected_to_host = godot::api->godot_method_bind_get_method("StreamPeerTCP", "is_connected_to_host");
	___mb.mb_set_no_delay = godot::api->godot_method_bind_get_method("StreamPeerTCP", "set_no_delay");
}

StreamPeerTCP *StreamPeerTCP::_new()
{
	return (StreamPeerTCP *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"StreamPeerTCP")());
}
Error StreamPeerTCP::connect_to_host(const String host, const int64_t port) {
	return (Error) ___godot_icall_int_String_int(___mb.mb_connect_to_host, (const Object *) this, host, port);
}

void StreamPeerTCP::disconnect_from_host() {
	___godot_icall_void(___mb.mb_disconnect_from_host, (const Object *) this);
}

String StreamPeerTCP::get_connected_host() const {
	return ___godot_icall_String(___mb.mb_get_connected_host, (const Object *) this);
}

int64_t StreamPeerTCP::get_connected_port() const {
	return ___godot_icall_int(___mb.mb_get_connected_port, (const Object *) this);
}

StreamPeerTCP::Status StreamPeerTCP::get_status() {
	return (StreamPeerTCP::Status) ___godot_icall_int(___mb.mb_get_status, (const Object *) this);
}

bool StreamPeerTCP::is_connected_to_host() const {
	return ___godot_icall_bool(___mb.mb_is_connected_to_host, (const Object *) this);
}

void StreamPeerTCP::set_no_delay(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_no_delay, (const Object *) this, enabled);
}

}