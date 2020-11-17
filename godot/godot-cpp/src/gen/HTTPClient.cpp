#include "HTTPClient.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "StreamPeer.hpp"


namespace godot {


HTTPClient::___method_bindings HTTPClient::___mb = {};

void HTTPClient::___init_method_bindings() {
	___mb.mb_close = godot::api->godot_method_bind_get_method("HTTPClient", "close");
	___mb.mb_connect_to_host = godot::api->godot_method_bind_get_method("HTTPClient", "connect_to_host");
	___mb.mb_get_connection = godot::api->godot_method_bind_get_method("HTTPClient", "get_connection");
	___mb.mb_get_read_chunk_size = godot::api->godot_method_bind_get_method("HTTPClient", "get_read_chunk_size");
	___mb.mb_get_response_body_length = godot::api->godot_method_bind_get_method("HTTPClient", "get_response_body_length");
	___mb.mb_get_response_code = godot::api->godot_method_bind_get_method("HTTPClient", "get_response_code");
	___mb.mb_get_response_headers = godot::api->godot_method_bind_get_method("HTTPClient", "get_response_headers");
	___mb.mb_get_response_headers_as_dictionary = godot::api->godot_method_bind_get_method("HTTPClient", "get_response_headers_as_dictionary");
	___mb.mb_get_status = godot::api->godot_method_bind_get_method("HTTPClient", "get_status");
	___mb.mb_has_response = godot::api->godot_method_bind_get_method("HTTPClient", "has_response");
	___mb.mb_is_blocking_mode_enabled = godot::api->godot_method_bind_get_method("HTTPClient", "is_blocking_mode_enabled");
	___mb.mb_is_response_chunked = godot::api->godot_method_bind_get_method("HTTPClient", "is_response_chunked");
	___mb.mb_poll = godot::api->godot_method_bind_get_method("HTTPClient", "poll");
	___mb.mb_query_string_from_dict = godot::api->godot_method_bind_get_method("HTTPClient", "query_string_from_dict");
	___mb.mb_read_response_body_chunk = godot::api->godot_method_bind_get_method("HTTPClient", "read_response_body_chunk");
	___mb.mb_request = godot::api->godot_method_bind_get_method("HTTPClient", "request");
	___mb.mb_request_raw = godot::api->godot_method_bind_get_method("HTTPClient", "request_raw");
	___mb.mb_set_blocking_mode = godot::api->godot_method_bind_get_method("HTTPClient", "set_blocking_mode");
	___mb.mb_set_connection = godot::api->godot_method_bind_get_method("HTTPClient", "set_connection");
	___mb.mb_set_read_chunk_size = godot::api->godot_method_bind_get_method("HTTPClient", "set_read_chunk_size");
}

HTTPClient *HTTPClient::_new()
{
	return (HTTPClient *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"HTTPClient")());
}
void HTTPClient::close() {
	___godot_icall_void(___mb.mb_close, (const Object *) this);
}

Error HTTPClient::connect_to_host(const String host, const int64_t port, const bool use_ssl, const bool verify_host) {
	return (Error) ___godot_icall_int_String_int_bool_bool(___mb.mb_connect_to_host, (const Object *) this, host, port, use_ssl, verify_host);
}

Ref<StreamPeer> HTTPClient::get_connection() const {
	return Ref<StreamPeer>::__internal_constructor(___godot_icall_Object(___mb.mb_get_connection, (const Object *) this));
}

int64_t HTTPClient::get_read_chunk_size() const {
	return ___godot_icall_int(___mb.mb_get_read_chunk_size, (const Object *) this);
}

int64_t HTTPClient::get_response_body_length() const {
	return ___godot_icall_int(___mb.mb_get_response_body_length, (const Object *) this);
}

int64_t HTTPClient::get_response_code() const {
	return ___godot_icall_int(___mb.mb_get_response_code, (const Object *) this);
}

PoolStringArray HTTPClient::get_response_headers() {
	return ___godot_icall_PoolStringArray(___mb.mb_get_response_headers, (const Object *) this);
}

Dictionary HTTPClient::get_response_headers_as_dictionary() {
	return ___godot_icall_Dictionary(___mb.mb_get_response_headers_as_dictionary, (const Object *) this);
}

HTTPClient::Status HTTPClient::get_status() const {
	return (HTTPClient::Status) ___godot_icall_int(___mb.mb_get_status, (const Object *) this);
}

bool HTTPClient::has_response() const {
	return ___godot_icall_bool(___mb.mb_has_response, (const Object *) this);
}

bool HTTPClient::is_blocking_mode_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_blocking_mode_enabled, (const Object *) this);
}

bool HTTPClient::is_response_chunked() const {
	return ___godot_icall_bool(___mb.mb_is_response_chunked, (const Object *) this);
}

Error HTTPClient::poll() {
	return (Error) ___godot_icall_int(___mb.mb_poll, (const Object *) this);
}

String HTTPClient::query_string_from_dict(const Dictionary fields) {
	return ___godot_icall_String_Dictionary(___mb.mb_query_string_from_dict, (const Object *) this, fields);
}

PoolByteArray HTTPClient::read_response_body_chunk() {
	return ___godot_icall_PoolByteArray(___mb.mb_read_response_body_chunk, (const Object *) this);
}

Error HTTPClient::request(const int64_t method, const String url, const PoolStringArray headers, const String body) {
	return (Error) ___godot_icall_int_int_String_PoolStringArray_String(___mb.mb_request, (const Object *) this, method, url, headers, body);
}

Error HTTPClient::request_raw(const int64_t method, const String url, const PoolStringArray headers, const PoolByteArray body) {
	return (Error) ___godot_icall_int_int_String_PoolStringArray_PoolByteArray(___mb.mb_request_raw, (const Object *) this, method, url, headers, body);
}

void HTTPClient::set_blocking_mode(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_blocking_mode, (const Object *) this, enabled);
}

void HTTPClient::set_connection(const Ref<StreamPeer> connection) {
	___godot_icall_void_Object(___mb.mb_set_connection, (const Object *) this, connection.ptr());
}

void HTTPClient::set_read_chunk_size(const int64_t bytes) {
	___godot_icall_void_int(___mb.mb_set_read_chunk_size, (const Object *) this, bytes);
}

}