#include "PacketPeerStream.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "StreamPeer.hpp"


namespace godot {


PacketPeerStream::___method_bindings PacketPeerStream::___mb = {};

void PacketPeerStream::___init_method_bindings() {
	___mb.mb_get_input_buffer_max_size = godot::api->godot_method_bind_get_method("PacketPeerStream", "get_input_buffer_max_size");
	___mb.mb_get_output_buffer_max_size = godot::api->godot_method_bind_get_method("PacketPeerStream", "get_output_buffer_max_size");
	___mb.mb_get_stream_peer = godot::api->godot_method_bind_get_method("PacketPeerStream", "get_stream_peer");
	___mb.mb_set_input_buffer_max_size = godot::api->godot_method_bind_get_method("PacketPeerStream", "set_input_buffer_max_size");
	___mb.mb_set_output_buffer_max_size = godot::api->godot_method_bind_get_method("PacketPeerStream", "set_output_buffer_max_size");
	___mb.mb_set_stream_peer = godot::api->godot_method_bind_get_method("PacketPeerStream", "set_stream_peer");
}

PacketPeerStream *PacketPeerStream::_new()
{
	return (PacketPeerStream *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PacketPeerStream")());
}
int64_t PacketPeerStream::get_input_buffer_max_size() const {
	return ___godot_icall_int(___mb.mb_get_input_buffer_max_size, (const Object *) this);
}

int64_t PacketPeerStream::get_output_buffer_max_size() const {
	return ___godot_icall_int(___mb.mb_get_output_buffer_max_size, (const Object *) this);
}

Ref<StreamPeer> PacketPeerStream::get_stream_peer() const {
	return Ref<StreamPeer>::__internal_constructor(___godot_icall_Object(___mb.mb_get_stream_peer, (const Object *) this));
}

void PacketPeerStream::set_input_buffer_max_size(const int64_t max_size_bytes) {
	___godot_icall_void_int(___mb.mb_set_input_buffer_max_size, (const Object *) this, max_size_bytes);
}

void PacketPeerStream::set_output_buffer_max_size(const int64_t max_size_bytes) {
	___godot_icall_void_int(___mb.mb_set_output_buffer_max_size, (const Object *) this, max_size_bytes);
}

void PacketPeerStream::set_stream_peer(const Ref<StreamPeer> peer) {
	___godot_icall_void_Object(___mb.mb_set_stream_peer, (const Object *) this, peer.ptr());
}

}