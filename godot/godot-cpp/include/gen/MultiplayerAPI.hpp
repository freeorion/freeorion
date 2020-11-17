#ifndef GODOT_CPP_MULTIPLAYERAPI_HPP
#define GODOT_CPP_MULTIPLAYERAPI_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class NetworkedMultiplayerPeer;
class Node;

class MultiplayerAPI : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__add_peer;
		godot_method_bind *mb__connected_to_server;
		godot_method_bind *mb__connection_failed;
		godot_method_bind *mb__del_peer;
		godot_method_bind *mb__server_disconnected;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_get_network_connected_peers;
		godot_method_bind *mb_get_network_peer;
		godot_method_bind *mb_get_network_unique_id;
		godot_method_bind *mb_get_rpc_sender_id;
		godot_method_bind *mb_has_network_peer;
		godot_method_bind *mb_is_network_server;
		godot_method_bind *mb_is_object_decoding_allowed;
		godot_method_bind *mb_is_refusing_new_network_connections;
		godot_method_bind *mb_poll;
		godot_method_bind *mb_send_bytes;
		godot_method_bind *mb_set_allow_object_decoding;
		godot_method_bind *mb_set_network_peer;
		godot_method_bind *mb_set_refuse_new_network_connections;
		godot_method_bind *mb_set_root_node;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "MultiplayerAPI"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum RPCMode {
		RPC_MODE_DISABLED = 0,
		RPC_MODE_REMOTE = 1,
		RPC_MODE_MASTER = 2,
		RPC_MODE_PUPPET = 3,
		RPC_MODE_SLAVE = 3,
		RPC_MODE_REMOTESYNC = 4,
		RPC_MODE_SYNC = 4,
		RPC_MODE_MASTERSYNC = 5,
		RPC_MODE_PUPPETSYNC = 6,
	};

	// constants


	static MultiplayerAPI *_new();

	// methods
	void _add_peer(const int64_t id);
	void _connected_to_server();
	void _connection_failed();
	void _del_peer(const int64_t id);
	void _server_disconnected();
	void clear();
	PoolIntArray get_network_connected_peers() const;
	Ref<NetworkedMultiplayerPeer> get_network_peer() const;
	int64_t get_network_unique_id() const;
	int64_t get_rpc_sender_id() const;
	bool has_network_peer() const;
	bool is_network_server() const;
	bool is_object_decoding_allowed() const;
	bool is_refusing_new_network_connections() const;
	void poll();
	Error send_bytes(const PoolByteArray bytes, const int64_t id = 0, const int64_t mode = 2);
	void set_allow_object_decoding(const bool enable);
	void set_network_peer(const Ref<NetworkedMultiplayerPeer> peer);
	void set_refuse_new_network_connections(const bool refuse);
	void set_root_node(const Node *node);

};

}

#endif