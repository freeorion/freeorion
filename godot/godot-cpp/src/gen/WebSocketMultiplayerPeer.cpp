#include "WebSocketMultiplayerPeer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "WebSocketPeer.hpp"


namespace godot {


WebSocketMultiplayerPeer::___method_bindings WebSocketMultiplayerPeer::___mb = {};

void WebSocketMultiplayerPeer::___init_method_bindings() {
	___mb.mb_get_peer = godot::api->godot_method_bind_get_method("WebSocketMultiplayerPeer", "get_peer");
	___mb.mb_set_buffers = godot::api->godot_method_bind_get_method("WebSocketMultiplayerPeer", "set_buffers");
}

Ref<WebSocketPeer> WebSocketMultiplayerPeer::get_peer(const int64_t peer_id) const {
	return Ref<WebSocketPeer>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_peer, (const Object *) this, peer_id));
}

Error WebSocketMultiplayerPeer::set_buffers(const int64_t input_buffer_size_kb, const int64_t input_max_packets, const int64_t output_buffer_size_kb, const int64_t output_max_packets) {
	return (Error) ___godot_icall_int_int_int_int_int(___mb.mb_set_buffers, (const Object *) this, input_buffer_size_kb, input_max_packets, output_buffer_size_kb, output_max_packets);
}

}