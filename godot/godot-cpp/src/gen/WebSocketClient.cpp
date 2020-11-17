#include "WebSocketClient.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "X509Certificate.hpp"


namespace godot {


WebSocketClient::___method_bindings WebSocketClient::___mb = {};

void WebSocketClient::___init_method_bindings() {
	___mb.mb_connect_to_url = godot::api->godot_method_bind_get_method("WebSocketClient", "connect_to_url");
	___mb.mb_disconnect_from_host = godot::api->godot_method_bind_get_method("WebSocketClient", "disconnect_from_host");
	___mb.mb_get_connected_host = godot::api->godot_method_bind_get_method("WebSocketClient", "get_connected_host");
	___mb.mb_get_connected_port = godot::api->godot_method_bind_get_method("WebSocketClient", "get_connected_port");
	___mb.mb_get_trusted_ssl_certificate = godot::api->godot_method_bind_get_method("WebSocketClient", "get_trusted_ssl_certificate");
	___mb.mb_is_verify_ssl_enabled = godot::api->godot_method_bind_get_method("WebSocketClient", "is_verify_ssl_enabled");
	___mb.mb_set_trusted_ssl_certificate = godot::api->godot_method_bind_get_method("WebSocketClient", "set_trusted_ssl_certificate");
	___mb.mb_set_verify_ssl_enabled = godot::api->godot_method_bind_get_method("WebSocketClient", "set_verify_ssl_enabled");
}

WebSocketClient *WebSocketClient::_new()
{
	return (WebSocketClient *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"WebSocketClient")());
}
Error WebSocketClient::connect_to_url(const String url, const PoolStringArray protocols, const bool gd_mp_api, const PoolStringArray custom_headers) {
	return (Error) ___godot_icall_int_String_PoolStringArray_bool_PoolStringArray(___mb.mb_connect_to_url, (const Object *) this, url, protocols, gd_mp_api, custom_headers);
}

void WebSocketClient::disconnect_from_host(const int64_t code, const String reason) {
	___godot_icall_void_int_String(___mb.mb_disconnect_from_host, (const Object *) this, code, reason);
}

String WebSocketClient::get_connected_host() const {
	return ___godot_icall_String(___mb.mb_get_connected_host, (const Object *) this);
}

int64_t WebSocketClient::get_connected_port() const {
	return ___godot_icall_int(___mb.mb_get_connected_port, (const Object *) this);
}

Ref<X509Certificate> WebSocketClient::get_trusted_ssl_certificate() const {
	return Ref<X509Certificate>::__internal_constructor(___godot_icall_Object(___mb.mb_get_trusted_ssl_certificate, (const Object *) this));
}

bool WebSocketClient::is_verify_ssl_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_verify_ssl_enabled, (const Object *) this);
}

void WebSocketClient::set_trusted_ssl_certificate(const Ref<X509Certificate> arg0) {
	___godot_icall_void_Object(___mb.mb_set_trusted_ssl_certificate, (const Object *) this, arg0.ptr());
}

void WebSocketClient::set_verify_ssl_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_verify_ssl_enabled, (const Object *) this, enabled);
}

}