#ifndef GODOT_CPP_NETWORKEDMULTIPLAYERENET_HPP
#define GODOT_CPP_NETWORKEDMULTIPLAYERENET_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "NetworkedMultiplayerENet.hpp"

#include "NetworkedMultiplayerPeer.hpp"
namespace godot {


class NetworkedMultiplayerENet : public NetworkedMultiplayerPeer {
	struct ___method_bindings {
		godot_method_bind *mb_close_connection;
		godot_method_bind *mb_create_client;
		godot_method_bind *mb_create_server;
		godot_method_bind *mb_disconnect_peer;
		godot_method_bind *mb_get_channel_count;
		godot_method_bind *mb_get_compression_mode;
		godot_method_bind *mb_get_last_packet_channel;
		godot_method_bind *mb_get_packet_channel;
		godot_method_bind *mb_get_peer_address;
		godot_method_bind *mb_get_peer_port;
		godot_method_bind *mb_get_transfer_channel;
		godot_method_bind *mb_is_always_ordered;
		godot_method_bind *mb_is_server_relay_enabled;
		godot_method_bind *mb_set_always_ordered;
		godot_method_bind *mb_set_bind_ip;
		godot_method_bind *mb_set_channel_count;
		godot_method_bind *mb_set_compression_mode;
		godot_method_bind *mb_set_server_relay_enabled;
		godot_method_bind *mb_set_transfer_channel;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "NetworkedMultiplayerENet"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum CompressionMode {
		COMPRESS_NONE = 0,
		COMPRESS_RANGE_CODER = 1,
		COMPRESS_FASTLZ = 2,
		COMPRESS_ZLIB = 3,
		COMPRESS_ZSTD = 4,
	};

	// constants


	static NetworkedMultiplayerENet *_new();

	// methods
	void close_connection(const int64_t wait_usec = 100);
	Error create_client(const String address, const int64_t port, const int64_t in_bandwidth = 0, const int64_t out_bandwidth = 0, const int64_t client_port = 0);
	Error create_server(const int64_t port, const int64_t max_clients = 32, const int64_t in_bandwidth = 0, const int64_t out_bandwidth = 0);
	void disconnect_peer(const int64_t id, const bool now = false);
	int64_t get_channel_count() const;
	NetworkedMultiplayerENet::CompressionMode get_compression_mode() const;
	int64_t get_last_packet_channel() const;
	int64_t get_packet_channel() const;
	String get_peer_address(const int64_t id) const;
	int64_t get_peer_port(const int64_t id) const;
	int64_t get_transfer_channel() const;
	bool is_always_ordered() const;
	bool is_server_relay_enabled() const;
	void set_always_ordered(const bool ordered);
	void set_bind_ip(const String ip);
	void set_channel_count(const int64_t channels);
	void set_compression_mode(const int64_t mode);
	void set_server_relay_enabled(const bool enabled);
	void set_transfer_channel(const int64_t channel);

};

}

#endif