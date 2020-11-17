#include "PacketPeer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PacketPeer::___method_bindings PacketPeer::___mb = {};

void PacketPeer::___init_method_bindings() {
	___mb.mb_get_available_packet_count = godot::api->godot_method_bind_get_method("PacketPeer", "get_available_packet_count");
	___mb.mb_get_encode_buffer_max_size = godot::api->godot_method_bind_get_method("PacketPeer", "get_encode_buffer_max_size");
	___mb.mb_get_packet = godot::api->godot_method_bind_get_method("PacketPeer", "get_packet");
	___mb.mb_get_packet_error = godot::api->godot_method_bind_get_method("PacketPeer", "get_packet_error");
	___mb.mb_get_var = godot::api->godot_method_bind_get_method("PacketPeer", "get_var");
	___mb.mb_is_object_decoding_allowed = godot::api->godot_method_bind_get_method("PacketPeer", "is_object_decoding_allowed");
	___mb.mb_put_packet = godot::api->godot_method_bind_get_method("PacketPeer", "put_packet");
	___mb.mb_put_var = godot::api->godot_method_bind_get_method("PacketPeer", "put_var");
	___mb.mb_set_allow_object_decoding = godot::api->godot_method_bind_get_method("PacketPeer", "set_allow_object_decoding");
	___mb.mb_set_encode_buffer_max_size = godot::api->godot_method_bind_get_method("PacketPeer", "set_encode_buffer_max_size");
}

int64_t PacketPeer::get_available_packet_count() const {
	return ___godot_icall_int(___mb.mb_get_available_packet_count, (const Object *) this);
}

int64_t PacketPeer::get_encode_buffer_max_size() const {
	return ___godot_icall_int(___mb.mb_get_encode_buffer_max_size, (const Object *) this);
}

PoolByteArray PacketPeer::get_packet() {
	return ___godot_icall_PoolByteArray(___mb.mb_get_packet, (const Object *) this);
}

Error PacketPeer::get_packet_error() const {
	return (Error) ___godot_icall_int(___mb.mb_get_packet_error, (const Object *) this);
}

Variant PacketPeer::get_var(const bool allow_objects) {
	return ___godot_icall_Variant_bool(___mb.mb_get_var, (const Object *) this, allow_objects);
}

bool PacketPeer::is_object_decoding_allowed() const {
	return ___godot_icall_bool(___mb.mb_is_object_decoding_allowed, (const Object *) this);
}

Error PacketPeer::put_packet(const PoolByteArray buffer) {
	return (Error) ___godot_icall_int_PoolByteArray(___mb.mb_put_packet, (const Object *) this, buffer);
}

Error PacketPeer::put_var(const Variant var, const bool full_objects) {
	return (Error) ___godot_icall_int_Variant_bool(___mb.mb_put_var, (const Object *) this, var, full_objects);
}

void PacketPeer::set_allow_object_decoding(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_allow_object_decoding, (const Object *) this, enable);
}

void PacketPeer::set_encode_buffer_max_size(const int64_t max_size) {
	___godot_icall_void_int(___mb.mb_set_encode_buffer_max_size, (const Object *) this, max_size);
}

}