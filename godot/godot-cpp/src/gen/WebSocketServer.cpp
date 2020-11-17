#include "WebSocketServer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "X509Certificate.hpp"
#include "CryptoKey.hpp"


namespace godot {


WebSocketServer::___method_bindings WebSocketServer::___mb = {};

void WebSocketServer::___init_method_bindings() {
	___mb.mb_disconnect_peer = godot::api->godot_method_bind_get_method("WebSocketServer", "disconnect_peer");
	___mb.mb_get_bind_ip = godot::api->godot_method_bind_get_method("WebSocketServer", "get_bind_ip");
	___mb.mb_get_ca_chain = godot::api->godot_method_bind_get_method("WebSocketServer", "get_ca_chain");
	___mb.mb_get_peer_address = godot::api->godot_method_bind_get_method("WebSocketServer", "get_peer_address");
	___mb.mb_get_peer_port = godot::api->godot_method_bind_get_method("WebSocketServer", "get_peer_port");
	___mb.mb_get_private_key = godot::api->godot_method_bind_get_method("WebSocketServer", "get_private_key");
	___mb.mb_get_ssl_certificate = godot::api->godot_method_bind_get_method("WebSocketServer", "get_ssl_certificate");
	___mb.mb_has_peer = godot::api->godot_method_bind_get_method("WebSocketServer", "has_peer");
	___mb.mb_is_listening = godot::api->godot_method_bind_get_method("WebSocketServer", "is_listening");
	___mb.mb_listen = godot::api->godot_method_bind_get_method("WebSocketServer", "listen");
	___mb.mb_set_bind_ip = godot::api->godot_method_bind_get_method("WebSocketServer", "set_bind_ip");
	___mb.mb_set_ca_chain = godot::api->godot_method_bind_get_method("WebSocketServer", "set_ca_chain");
	___mb.mb_set_private_key = godot::api->godot_method_bind_get_method("WebSocketServer", "set_private_key");
	___mb.mb_set_ssl_certificate = godot::api->godot_method_bind_get_method("WebSocketServer", "set_ssl_certificate");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("WebSocketServer", "stop");
}

WebSocketServer *WebSocketServer::_new()
{
	return (WebSocketServer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"WebSocketServer")());
}
void WebSocketServer::disconnect_peer(const int64_t id, const int64_t code, const String reason) {
	___godot_icall_void_int_int_String(___mb.mb_disconnect_peer, (const Object *) this, id, code, reason);
}

String WebSocketServer::get_bind_ip() const {
	return ___godot_icall_String(___mb.mb_get_bind_ip, (const Object *) this);
}

Ref<X509Certificate> WebSocketServer::get_ca_chain() const {
	return Ref<X509Certificate>::__internal_constructor(___godot_icall_Object(___mb.mb_get_ca_chain, (const Object *) this));
}

String WebSocketServer::get_peer_address(const int64_t id) const {
	return ___godot_icall_String_int(___mb.mb_get_peer_address, (const Object *) this, id);
}

int64_t WebSocketServer::get_peer_port(const int64_t id) const {
	return ___godot_icall_int_int(___mb.mb_get_peer_port, (const Object *) this, id);
}

Ref<CryptoKey> WebSocketServer::get_private_key() const {
	return Ref<CryptoKey>::__internal_constructor(___godot_icall_Object(___mb.mb_get_private_key, (const Object *) this));
}

Ref<X509Certificate> WebSocketServer::get_ssl_certificate() const {
	return Ref<X509Certificate>::__internal_constructor(___godot_icall_Object(___mb.mb_get_ssl_certificate, (const Object *) this));
}

bool WebSocketServer::has_peer(const int64_t id) const {
	return ___godot_icall_bool_int(___mb.mb_has_peer, (const Object *) this, id);
}

bool WebSocketServer::is_listening() const {
	return ___godot_icall_bool(___mb.mb_is_listening, (const Object *) this);
}

Error WebSocketServer::listen(const int64_t port, const PoolStringArray protocols, const bool gd_mp_api) {
	return (Error) ___godot_icall_int_int_PoolStringArray_bool(___mb.mb_listen, (const Object *) this, port, protocols, gd_mp_api);
}

void WebSocketServer::set_bind_ip(const String arg0) {
	___godot_icall_void_String(___mb.mb_set_bind_ip, (const Object *) this, arg0);
}

void WebSocketServer::set_ca_chain(const Ref<X509Certificate> arg0) {
	___godot_icall_void_Object(___mb.mb_set_ca_chain, (const Object *) this, arg0.ptr());
}

void WebSocketServer::set_private_key(const Ref<CryptoKey> arg0) {
	___godot_icall_void_Object(___mb.mb_set_private_key, (const Object *) this, arg0.ptr());
}

void WebSocketServer::set_ssl_certificate(const Ref<X509Certificate> arg0) {
	___godot_icall_void_Object(___mb.mb_set_ssl_certificate, (const Object *) this, arg0.ptr());
}

void WebSocketServer::stop() {
	___godot_icall_void(___mb.mb_stop, (const Object *) this);
}

}