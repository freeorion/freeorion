#ifndef GODOT_CPP_STREAMPEERTCP_HPP
#define GODOT_CPP_STREAMPEERTCP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "StreamPeerTCP.hpp"

#include "StreamPeer.hpp"
namespace godot {


class StreamPeerTCP : public StreamPeer {
	struct ___method_bindings {
		godot_method_bind *mb_connect_to_host;
		godot_method_bind *mb_disconnect_from_host;
		godot_method_bind *mb_get_connected_host;
		godot_method_bind *mb_get_connected_port;
		godot_method_bind *mb_get_status;
		godot_method_bind *mb_is_connected_to_host;
		godot_method_bind *mb_set_no_delay;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "StreamPeerTCP"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Status {
		STATUS_NONE = 0,
		STATUS_CONNECTING = 1,
		STATUS_CONNECTED = 2,
		STATUS_ERROR = 3,
	};

	// constants


	static StreamPeerTCP *_new();

	// methods
	Error connect_to_host(const String host, const int64_t port);
	void disconnect_from_host();
	String get_connected_host() const;
	int64_t get_connected_port() const;
	StreamPeerTCP::Status get_status();
	bool is_connected_to_host() const;
	void set_no_delay(const bool enabled);

};

}

#endif