#ifndef GODOT_CPP_WEBSOCKETCLIENT_HPP
#define GODOT_CPP_WEBSOCKETCLIENT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "WebSocketMultiplayerPeer.hpp"
namespace godot {

class X509Certificate;

class WebSocketClient : public WebSocketMultiplayerPeer {
	struct ___method_bindings {
		godot_method_bind *mb_connect_to_url;
		godot_method_bind *mb_disconnect_from_host;
		godot_method_bind *mb_get_connected_host;
		godot_method_bind *mb_get_connected_port;
		godot_method_bind *mb_get_trusted_ssl_certificate;
		godot_method_bind *mb_is_verify_ssl_enabled;
		godot_method_bind *mb_set_trusted_ssl_certificate;
		godot_method_bind *mb_set_verify_ssl_enabled;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "WebSocketClient"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static WebSocketClient *_new();

	// methods
	Error connect_to_url(const String url, const PoolStringArray protocols = PoolStringArray(), const bool gd_mp_api = false, const PoolStringArray custom_headers = PoolStringArray());
	void disconnect_from_host(const int64_t code = 1000, const String reason = "");
	String get_connected_host() const;
	int64_t get_connected_port() const;
	Ref<X509Certificate> get_trusted_ssl_certificate() const;
	bool is_verify_ssl_enabled() const;
	void set_trusted_ssl_certificate(const Ref<X509Certificate> arg0);
	void set_verify_ssl_enabled(const bool enabled);

};

}

#endif