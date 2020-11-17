#ifndef GODOT_CPP_HTTPCLIENT_HPP
#define GODOT_CPP_HTTPCLIENT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "HTTPClient.hpp"

#include "Reference.hpp"
namespace godot {

class StreamPeer;

class HTTPClient : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_close;
		godot_method_bind *mb_connect_to_host;
		godot_method_bind *mb_get_connection;
		godot_method_bind *mb_get_read_chunk_size;
		godot_method_bind *mb_get_response_body_length;
		godot_method_bind *mb_get_response_code;
		godot_method_bind *mb_get_response_headers;
		godot_method_bind *mb_get_response_headers_as_dictionary;
		godot_method_bind *mb_get_status;
		godot_method_bind *mb_has_response;
		godot_method_bind *mb_is_blocking_mode_enabled;
		godot_method_bind *mb_is_response_chunked;
		godot_method_bind *mb_poll;
		godot_method_bind *mb_query_string_from_dict;
		godot_method_bind *mb_read_response_body_chunk;
		godot_method_bind *mb_request;
		godot_method_bind *mb_request_raw;
		godot_method_bind *mb_set_blocking_mode;
		godot_method_bind *mb_set_connection;
		godot_method_bind *mb_set_read_chunk_size;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "HTTPClient"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Status {
		STATUS_DISCONNECTED = 0,
		STATUS_RESOLVING = 1,
		STATUS_CANT_RESOLVE = 2,
		STATUS_CONNECTING = 3,
		STATUS_CANT_CONNECT = 4,
		STATUS_CONNECTED = 5,
		STATUS_REQUESTING = 6,
		STATUS_BODY = 7,
		STATUS_CONNECTION_ERROR = 8,
		STATUS_SSL_HANDSHAKE_ERROR = 9,
	};
	enum Method {
		METHOD_GET = 0,
		METHOD_HEAD = 1,
		METHOD_POST = 2,
		METHOD_PUT = 3,
		METHOD_DELETE = 4,
		METHOD_OPTIONS = 5,
		METHOD_TRACE = 6,
		METHOD_CONNECT = 7,
		METHOD_PATCH = 8,
		METHOD_MAX = 9,
	};
	enum ResponseCode {
		RESPONSE_CONTINUE = 100,
		RESPONSE_SWITCHING_PROTOCOLS = 101,
		RESPONSE_PROCESSING = 102,
		RESPONSE_OK = 200,
		RESPONSE_CREATED = 201,
		RESPONSE_ACCEPTED = 202,
		RESPONSE_NON_AUTHORITATIVE_INFORMATION = 203,
		RESPONSE_NO_CONTENT = 204,
		RESPONSE_RESET_CONTENT = 205,
		RESPONSE_PARTIAL_CONTENT = 206,
		RESPONSE_MULTI_STATUS = 207,
		RESPONSE_ALREADY_REPORTED = 208,
		RESPONSE_IM_USED = 226,
		RESPONSE_MULTIPLE_CHOICES = 300,
		RESPONSE_MOVED_PERMANENTLY = 301,
		RESPONSE_FOUND = 302,
		RESPONSE_SEE_OTHER = 303,
		RESPONSE_NOT_MODIFIED = 304,
		RESPONSE_USE_PROXY = 305,
		RESPONSE_SWITCH_PROXY = 306,
		RESPONSE_TEMPORARY_REDIRECT = 307,
		RESPONSE_PERMANENT_REDIRECT = 308,
		RESPONSE_BAD_REQUEST = 400,
		RESPONSE_UNAUTHORIZED = 401,
		RESPONSE_PAYMENT_REQUIRED = 402,
		RESPONSE_FORBIDDEN = 403,
		RESPONSE_NOT_FOUND = 404,
		RESPONSE_METHOD_NOT_ALLOWED = 405,
		RESPONSE_NOT_ACCEPTABLE = 406,
		RESPONSE_PROXY_AUTHENTICATION_REQUIRED = 407,
		RESPONSE_REQUEST_TIMEOUT = 408,
		RESPONSE_CONFLICT = 409,
		RESPONSE_GONE = 410,
		RESPONSE_LENGTH_REQUIRED = 411,
		RESPONSE_PRECONDITION_FAILED = 412,
		RESPONSE_REQUEST_ENTITY_TOO_LARGE = 413,
		RESPONSE_REQUEST_URI_TOO_LONG = 414,
		RESPONSE_UNSUPPORTED_MEDIA_TYPE = 415,
		RESPONSE_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
		RESPONSE_EXPECTATION_FAILED = 417,
		RESPONSE_IM_A_TEAPOT = 418,
		RESPONSE_MISDIRECTED_REQUEST = 421,
		RESPONSE_UNPROCESSABLE_ENTITY = 422,
		RESPONSE_LOCKED = 423,
		RESPONSE_FAILED_DEPENDENCY = 424,
		RESPONSE_UPGRADE_REQUIRED = 426,
		RESPONSE_PRECONDITION_REQUIRED = 428,
		RESPONSE_TOO_MANY_REQUESTS = 429,
		RESPONSE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
		RESPONSE_UNAVAILABLE_FOR_LEGAL_REASONS = 451,
		RESPONSE_INTERNAL_SERVER_ERROR = 500,
		RESPONSE_NOT_IMPLEMENTED = 501,
		RESPONSE_BAD_GATEWAY = 502,
		RESPONSE_SERVICE_UNAVAILABLE = 503,
		RESPONSE_GATEWAY_TIMEOUT = 504,
		RESPONSE_HTTP_VERSION_NOT_SUPPORTED = 505,
		RESPONSE_VARIANT_ALSO_NEGOTIATES = 506,
		RESPONSE_INSUFFICIENT_STORAGE = 507,
		RESPONSE_LOOP_DETECTED = 508,
		RESPONSE_NOT_EXTENDED = 510,
		RESPONSE_NETWORK_AUTH_REQUIRED = 511,
	};

	// constants


	static HTTPClient *_new();

	// methods
	void close();
	Error connect_to_host(const String host, const int64_t port = -1, const bool use_ssl = false, const bool verify_host = true);
	Ref<StreamPeer> get_connection() const;
	int64_t get_read_chunk_size() const;
	int64_t get_response_body_length() const;
	int64_t get_response_code() const;
	PoolStringArray get_response_headers();
	Dictionary get_response_headers_as_dictionary();
	HTTPClient::Status get_status() const;
	bool has_response() const;
	bool is_blocking_mode_enabled() const;
	bool is_response_chunked() const;
	Error poll();
	String query_string_from_dict(const Dictionary fields);
	PoolByteArray read_response_body_chunk();
	Error request(const int64_t method, const String url, const PoolStringArray headers, const String body = "");
	Error request_raw(const int64_t method, const String url, const PoolStringArray headers, const PoolByteArray body);
	void set_blocking_mode(const bool enabled);
	void set_connection(const Ref<StreamPeer> connection);
	void set_read_chunk_size(const int64_t bytes);

};

}

#endif