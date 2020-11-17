#include "StreamPeerSSL.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "StreamPeer.hpp"
#include "CryptoKey.hpp"
#include "X509Certificate.hpp"


namespace godot {


StreamPeerSSL::___method_bindings StreamPeerSSL::___mb = {};

void StreamPeerSSL::___init_method_bindings() {
	___mb.mb_accept_stream = godot::api->godot_method_bind_get_method("StreamPeerSSL", "accept_stream");
	___mb.mb_connect_to_stream = godot::api->godot_method_bind_get_method("StreamPeerSSL", "connect_to_stream");
	___mb.mb_disconnect_from_stream = godot::api->godot_method_bind_get_method("StreamPeerSSL", "disconnect_from_stream");
	___mb.mb_get_status = godot::api->godot_method_bind_get_method("StreamPeerSSL", "get_status");
	___mb.mb_is_blocking_handshake_enabled = godot::api->godot_method_bind_get_method("StreamPeerSSL", "is_blocking_handshake_enabled");
	___mb.mb_poll = godot::api->godot_method_bind_get_method("StreamPeerSSL", "poll");
	___mb.mb_set_blocking_handshake_enabled = godot::api->godot_method_bind_get_method("StreamPeerSSL", "set_blocking_handshake_enabled");
}

StreamPeerSSL *StreamPeerSSL::_new()
{
	return (StreamPeerSSL *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"StreamPeerSSL")());
}
Error StreamPeerSSL::accept_stream(const Ref<StreamPeer> stream, const Ref<CryptoKey> private_key, const Ref<X509Certificate> certificate, const Ref<X509Certificate> chain) {
	return (Error) ___godot_icall_int_Object_Object_Object_Object(___mb.mb_accept_stream, (const Object *) this, stream.ptr(), private_key.ptr(), certificate.ptr(), chain.ptr());
}

Error StreamPeerSSL::connect_to_stream(const Ref<StreamPeer> stream, const bool validate_certs, const String for_hostname, const Ref<X509Certificate> valid_certificate) {
	return (Error) ___godot_icall_int_Object_bool_String_Object(___mb.mb_connect_to_stream, (const Object *) this, stream.ptr(), validate_certs, for_hostname, valid_certificate.ptr());
}

void StreamPeerSSL::disconnect_from_stream() {
	___godot_icall_void(___mb.mb_disconnect_from_stream, (const Object *) this);
}

StreamPeerSSL::Status StreamPeerSSL::get_status() const {
	return (StreamPeerSSL::Status) ___godot_icall_int(___mb.mb_get_status, (const Object *) this);
}

bool StreamPeerSSL::is_blocking_handshake_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_blocking_handshake_enabled, (const Object *) this);
}

void StreamPeerSSL::poll() {
	___godot_icall_void(___mb.mb_poll, (const Object *) this);
}

void StreamPeerSSL::set_blocking_handshake_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_blocking_handshake_enabled, (const Object *) this, enabled);
}

}