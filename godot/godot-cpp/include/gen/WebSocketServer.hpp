#ifndef GODOT_CPP_WEBSOCKETSERVER_HPP
#define GODOT_CPP_WEBSOCKETSERVER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "WebSocketMultiplayerPeer.hpp"
namespace godot {

class X509Certificate;
class CryptoKey;

class WebSocketServer : public WebSocketMultiplayerPeer {
	struct ___method_bindings {
		godot_method_bind *mb_disconnect_peer;
		godot_method_bind *mb_get_bind_ip;
		godot_method_bind *mb_get_ca_chain;
		godot_method_bind *mb_get_peer_address;
		godot_method_bind *mb_get_peer_port;
		godot_method_bind *mb_get_private_key;
		godot_method_bind *mb_get_ssl_certificate;
		godot_method_bind *mb_has_peer;
		godot_method_bind *mb_is_listening;
		godot_method_bind *mb_listen;
		godot_method_bind *mb_set_bind_ip;
		godot_method_bind *mb_set_ca_chain;
		godot_method_bind *mb_set_private_key;
		godot_method_bind *mb_set_ssl_certificate;
		godot_method_bind *mb_stop;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "WebSocketServer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static WebSocketServer *_new();

	// methods
	void disconnect_peer(const int64_t id, const int64_t code = 1000, const String reason = "");
	String get_bind_ip() const;
	Ref<X509Certificate> get_ca_chain() const;
	String get_peer_address(const int64_t id) const;
	int64_t get_peer_port(const int64_t id) const;
	Ref<CryptoKey> get_private_key() const;
	Ref<X509Certificate> get_ssl_certificate() const;
	bool has_peer(const int64_t id) const;
	bool is_listening() const;
	Error listen(const int64_t port, const PoolStringArray protocols = PoolStringArray(), const bool gd_mp_api = false);
	void set_bind_ip(const String arg0);
	void set_ca_chain(const Ref<X509Certificate> arg0);
	void set_private_key(const Ref<CryptoKey> arg0);
	void set_ssl_certificate(const Ref<X509Certificate> arg0);
	void stop();

};

}

#endif