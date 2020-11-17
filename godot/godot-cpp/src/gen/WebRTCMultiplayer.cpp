#include "WebRTCMultiplayer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "WebRTCPeerConnection.hpp"


namespace godot {


WebRTCMultiplayer::___method_bindings WebRTCMultiplayer::___mb = {};

void WebRTCMultiplayer::___init_method_bindings() {
	___mb.mb_add_peer = godot::api->godot_method_bind_get_method("WebRTCMultiplayer", "add_peer");
	___mb.mb_close = godot::api->godot_method_bind_get_method("WebRTCMultiplayer", "close");
	___mb.mb_get_peer = godot::api->godot_method_bind_get_method("WebRTCMultiplayer", "get_peer");
	___mb.mb_get_peers = godot::api->godot_method_bind_get_method("WebRTCMultiplayer", "get_peers");
	___mb.mb_has_peer = godot::api->godot_method_bind_get_method("WebRTCMultiplayer", "has_peer");
	___mb.mb_initialize = godot::api->godot_method_bind_get_method("WebRTCMultiplayer", "initialize");
	___mb.mb_remove_peer = godot::api->godot_method_bind_get_method("WebRTCMultiplayer", "remove_peer");
}

WebRTCMultiplayer *WebRTCMultiplayer::_new()
{
	return (WebRTCMultiplayer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"WebRTCMultiplayer")());
}
Error WebRTCMultiplayer::add_peer(const Ref<WebRTCPeerConnection> peer, const int64_t peer_id, const int64_t unreliable_lifetime) {
	return (Error) ___godot_icall_int_Object_int_int(___mb.mb_add_peer, (const Object *) this, peer.ptr(), peer_id, unreliable_lifetime);
}

void WebRTCMultiplayer::close() {
	___godot_icall_void(___mb.mb_close, (const Object *) this);
}

Dictionary WebRTCMultiplayer::get_peer(const int64_t peer_id) {
	return ___godot_icall_Dictionary_int(___mb.mb_get_peer, (const Object *) this, peer_id);
}

Dictionary WebRTCMultiplayer::get_peers() {
	return ___godot_icall_Dictionary(___mb.mb_get_peers, (const Object *) this);
}

bool WebRTCMultiplayer::has_peer(const int64_t peer_id) {
	return ___godot_icall_bool_int(___mb.mb_has_peer, (const Object *) this, peer_id);
}

Error WebRTCMultiplayer::initialize(const int64_t peer_id, const bool server_compatibility) {
	return (Error) ___godot_icall_int_int_bool(___mb.mb_initialize, (const Object *) this, peer_id, server_compatibility);
}

void WebRTCMultiplayer::remove_peer(const int64_t peer_id) {
	___godot_icall_void_int(___mb.mb_remove_peer, (const Object *) this, peer_id);
}

}