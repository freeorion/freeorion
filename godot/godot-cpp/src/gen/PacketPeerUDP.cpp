#include "PacketPeerUDP.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PacketPeerUDP::___method_bindings PacketPeerUDP::___mb = {};

void PacketPeerUDP::___init_method_bindings() {
	___mb.mb_close = godot::api->godot_method_bind_get_method("PacketPeerUDP", "close");
	___mb.mb_get_packet_ip = godot::api->godot_method_bind_get_method("PacketPeerUDP", "get_packet_ip");
	___mb.mb_get_packet_port = godot::api->godot_method_bind_get_method("PacketPeerUDP", "get_packet_port");
	___mb.mb_is_listening = godot::api->godot_method_bind_get_method("PacketPeerUDP", "is_listening");
	___mb.mb_join_multicast_group = godot::api->godot_method_bind_get_method("PacketPeerUDP", "join_multicast_group");
	___mb.mb_leave_multicast_group = godot::api->godot_method_bind_get_method("PacketPeerUDP", "leave_multicast_group");
	___mb.mb_listen = godot::api->godot_method_bind_get_method("PacketPeerUDP", "listen");
	___mb.mb_set_broadcast_enabled = godot::api->godot_method_bind_get_method("PacketPeerUDP", "set_broadcast_enabled");
	___mb.mb_set_dest_address = godot::api->godot_method_bind_get_method("PacketPeerUDP", "set_dest_address");
	___mb.mb_wait = godot::api->godot_method_bind_get_method("PacketPeerUDP", "wait");
}

PacketPeerUDP *PacketPeerUDP::_new()
{
	return (PacketPeerUDP *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PacketPeerUDP")());
}
void PacketPeerUDP::close() {
	___godot_icall_void(___mb.mb_close, (const Object *) this);
}

String PacketPeerUDP::get_packet_ip() const {
	return ___godot_icall_String(___mb.mb_get_packet_ip, (const Object *) this);
}

int64_t PacketPeerUDP::get_packet_port() const {
	return ___godot_icall_int(___mb.mb_get_packet_port, (const Object *) this);
}

bool PacketPeerUDP::is_listening() const {
	return ___godot_icall_bool(___mb.mb_is_listening, (const Object *) this);
}

Error PacketPeerUDP::join_multicast_group(const String multicast_address, const String interface_name) {
	return (Error) ___godot_icall_int_String_String(___mb.mb_join_multicast_group, (const Object *) this, multicast_address, interface_name);
}

Error PacketPeerUDP::leave_multicast_group(const String multicast_address, const String interface_name) {
	return (Error) ___godot_icall_int_String_String(___mb.mb_leave_multicast_group, (const Object *) this, multicast_address, interface_name);
}

Error PacketPeerUDP::listen(const int64_t port, const String bind_address, const int64_t recv_buf_size) {
	return (Error) ___godot_icall_int_int_String_int(___mb.mb_listen, (const Object *) this, port, bind_address, recv_buf_size);
}

void PacketPeerUDP::set_broadcast_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_broadcast_enabled, (const Object *) this, enabled);
}

Error PacketPeerUDP::set_dest_address(const String host, const int64_t port) {
	return (Error) ___godot_icall_int_String_int(___mb.mb_set_dest_address, (const Object *) this, host, port);
}

Error PacketPeerUDP::wait() {
	return (Error) ___godot_icall_int(___mb.mb_wait, (const Object *) this);
}

}