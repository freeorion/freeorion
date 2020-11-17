#include "HTTPRequest.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


HTTPRequest::___method_bindings HTTPRequest::___mb = {};

void HTTPRequest::___init_method_bindings() {
	___mb.mb__redirect_request = godot::api->godot_method_bind_get_method("HTTPRequest", "_redirect_request");
	___mb.mb__request_done = godot::api->godot_method_bind_get_method("HTTPRequest", "_request_done");
	___mb.mb__timeout = godot::api->godot_method_bind_get_method("HTTPRequest", "_timeout");
	___mb.mb_cancel_request = godot::api->godot_method_bind_get_method("HTTPRequest", "cancel_request");
	___mb.mb_get_body_size = godot::api->godot_method_bind_get_method("HTTPRequest", "get_body_size");
	___mb.mb_get_body_size_limit = godot::api->godot_method_bind_get_method("HTTPRequest", "get_body_size_limit");
	___mb.mb_get_download_chunk_size = godot::api->godot_method_bind_get_method("HTTPRequest", "get_download_chunk_size");
	___mb.mb_get_download_file = godot::api->godot_method_bind_get_method("HTTPRequest", "get_download_file");
	___mb.mb_get_downloaded_bytes = godot::api->godot_method_bind_get_method("HTTPRequest", "get_downloaded_bytes");
	___mb.mb_get_http_client_status = godot::api->godot_method_bind_get_method("HTTPRequest", "get_http_client_status");
	___mb.mb_get_max_redirects = godot::api->godot_method_bind_get_method("HTTPRequest", "get_max_redirects");
	___mb.mb_get_timeout = godot::api->godot_method_bind_get_method("HTTPRequest", "get_timeout");
	___mb.mb_is_using_threads = godot::api->godot_method_bind_get_method("HTTPRequest", "is_using_threads");
	___mb.mb_request = godot::api->godot_method_bind_get_method("HTTPRequest", "request");
	___mb.mb_set_body_size_limit = godot::api->godot_method_bind_get_method("HTTPRequest", "set_body_size_limit");
	___mb.mb_set_download_chunk_size = godot::api->godot_method_bind_get_method("HTTPRequest", "set_download_chunk_size");
	___mb.mb_set_download_file = godot::api->godot_method_bind_get_method("HTTPRequest", "set_download_file");
	___mb.mb_set_max_redirects = godot::api->godot_method_bind_get_method("HTTPRequest", "set_max_redirects");
	___mb.mb_set_timeout = godot::api->godot_method_bind_get_method("HTTPRequest", "set_timeout");
	___mb.mb_set_use_threads = godot::api->godot_method_bind_get_method("HTTPRequest", "set_use_threads");
}

HTTPRequest *HTTPRequest::_new()
{
	return (HTTPRequest *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"HTTPRequest")());
}
void HTTPRequest::_redirect_request(const String arg0) {
	___godot_icall_void_String(___mb.mb__redirect_request, (const Object *) this, arg0);
}

void HTTPRequest::_request_done(const int64_t arg0, const int64_t arg1, const PoolStringArray arg2, const PoolByteArray arg3) {
	___godot_icall_void_int_int_PoolStringArray_PoolByteArray(___mb.mb__request_done, (const Object *) this, arg0, arg1, arg2, arg3);
}

void HTTPRequest::_timeout() {
	___godot_icall_void(___mb.mb__timeout, (const Object *) this);
}

void HTTPRequest::cancel_request() {
	___godot_icall_void(___mb.mb_cancel_request, (const Object *) this);
}

int64_t HTTPRequest::get_body_size() const {
	return ___godot_icall_int(___mb.mb_get_body_size, (const Object *) this);
}

int64_t HTTPRequest::get_body_size_limit() const {
	return ___godot_icall_int(___mb.mb_get_body_size_limit, (const Object *) this);
}

int64_t HTTPRequest::get_download_chunk_size() const {
	return ___godot_icall_int(___mb.mb_get_download_chunk_size, (const Object *) this);
}

String HTTPRequest::get_download_file() const {
	return ___godot_icall_String(___mb.mb_get_download_file, (const Object *) this);
}

int64_t HTTPRequest::get_downloaded_bytes() const {
	return ___godot_icall_int(___mb.mb_get_downloaded_bytes, (const Object *) this);
}

HTTPClient::Status HTTPRequest::get_http_client_status() const {
	return (HTTPClient::Status) ___godot_icall_int(___mb.mb_get_http_client_status, (const Object *) this);
}

int64_t HTTPRequest::get_max_redirects() const {
	return ___godot_icall_int(___mb.mb_get_max_redirects, (const Object *) this);
}

int64_t HTTPRequest::get_timeout() {
	return ___godot_icall_int(___mb.mb_get_timeout, (const Object *) this);
}

bool HTTPRequest::is_using_threads() const {
	return ___godot_icall_bool(___mb.mb_is_using_threads, (const Object *) this);
}

Error HTTPRequest::request(const String url, const PoolStringArray custom_headers, const bool ssl_validate_domain, const int64_t method, const String request_data) {
	return (Error) ___godot_icall_int_String_PoolStringArray_bool_int_String(___mb.mb_request, (const Object *) this, url, custom_headers, ssl_validate_domain, method, request_data);
}

void HTTPRequest::set_body_size_limit(const int64_t bytes) {
	___godot_icall_void_int(___mb.mb_set_body_size_limit, (const Object *) this, bytes);
}

void HTTPRequest::set_download_chunk_size(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb_set_download_chunk_size, (const Object *) this, arg0);
}

void HTTPRequest::set_download_file(const String path) {
	___godot_icall_void_String(___mb.mb_set_download_file, (const Object *) this, path);
}

void HTTPRequest::set_max_redirects(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_max_redirects, (const Object *) this, amount);
}

void HTTPRequest::set_timeout(const int64_t timeout) {
	___godot_icall_void_int(___mb.mb_set_timeout, (const Object *) this, timeout);
}

void HTTPRequest::set_use_threads(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_threads, (const Object *) this, enable);
}

}