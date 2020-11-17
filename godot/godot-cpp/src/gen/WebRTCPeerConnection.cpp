#include "WebRTCPeerConnection.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "WebRTCDataChannel.hpp"


namespace godot {


WebRTCPeerConnection::___method_bindings WebRTCPeerConnection::___mb = {};

void WebRTCPeerConnection::___init_method_bindings() {
	___mb.mb_add_ice_candidate = godot::api->godot_method_bind_get_method("WebRTCPeerConnection", "add_ice_candidate");
	___mb.mb_close = godot::api->godot_method_bind_get_method("WebRTCPeerConnection", "close");
	___mb.mb_create_data_channel = godot::api->godot_method_bind_get_method("WebRTCPeerConnection", "create_data_channel");
	___mb.mb_create_offer = godot::api->godot_method_bind_get_method("WebRTCPeerConnection", "create_offer");
	___mb.mb_get_connection_state = godot::api->godot_method_bind_get_method("WebRTCPeerConnection", "get_connection_state");
	___mb.mb_initialize = godot::api->godot_method_bind_get_method("WebRTCPeerConnection", "initialize");
	___mb.mb_poll = godot::api->godot_method_bind_get_method("WebRTCPeerConnection", "poll");
	___mb.mb_set_local_description = godot::api->godot_method_bind_get_method("WebRTCPeerConnection", "set_local_description");
	___mb.mb_set_remote_description = godot::api->godot_method_bind_get_method("WebRTCPeerConnection", "set_remote_description");
}

WebRTCPeerConnection *WebRTCPeerConnection::_new()
{
	return (WebRTCPeerConnection *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"WebRTCPeerConnection")());
}
Error WebRTCPeerConnection::add_ice_candidate(const String media, const int64_t index, const String name) {
	return (Error) ___godot_icall_int_String_int_String(___mb.mb_add_ice_candidate, (const Object *) this, media, index, name);
}

void WebRTCPeerConnection::close() {
	___godot_icall_void(___mb.mb_close, (const Object *) this);
}

Ref<WebRTCDataChannel> WebRTCPeerConnection::create_data_channel(const String label, const Dictionary options) {
	return Ref<WebRTCDataChannel>::__internal_constructor(___godot_icall_Object_String_Dictionary(___mb.mb_create_data_channel, (const Object *) this, label, options));
}

Error WebRTCPeerConnection::create_offer() {
	return (Error) ___godot_icall_int(___mb.mb_create_offer, (const Object *) this);
}

WebRTCPeerConnection::ConnectionState WebRTCPeerConnection::get_connection_state() const {
	return (WebRTCPeerConnection::ConnectionState) ___godot_icall_int(___mb.mb_get_connection_state, (const Object *) this);
}

Error WebRTCPeerConnection::initialize(const Dictionary configuration) {
	return (Error) ___godot_icall_int_Dictionary(___mb.mb_initialize, (const Object *) this, configuration);
}

Error WebRTCPeerConnection::poll() {
	return (Error) ___godot_icall_int(___mb.mb_poll, (const Object *) this);
}

Error WebRTCPeerConnection::set_local_description(const String type, const String sdp) {
	return (Error) ___godot_icall_int_String_String(___mb.mb_set_local_description, (const Object *) this, type, sdp);
}

Error WebRTCPeerConnection::set_remote_description(const String type, const String sdp) {
	return (Error) ___godot_icall_int_String_String(___mb.mb_set_remote_description, (const Object *) this, type, sdp);
}

}