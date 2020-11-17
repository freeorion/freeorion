#ifndef GODOT_CPP_WEBSOCKETPEER_HPP
#define GODOT_CPP_WEBSOCKETPEER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "WebSocketPeer.hpp"

#include "PacketPeer.hpp"
namespace godot {


class WebSocketPeer : public PacketPeer {
	struct ___method_bindings {
		godot_method_bind *mb_close;
		godot_method_bind *mb_get_connected_host;
		godot_method_bind *mb_get_connected_port;
		godot_method_bind *mb_get_write_mode;
		godot_method_bind *mb_is_connected_to_host;
		godot_method_bind *mb_set_no_delay;
		godot_method_bind *mb_set_write_mode;
		godot_method_bind *mb_was_string_packet;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "WebSocketPeer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum WriteMode {
		WRITE_MODE_TEXT = 0,
		WRITE_MODE_BINARY = 1,
	};

	// constants


	static WebSocketPeer *_new();

	// methods
	void close(const int64_t code = 1000, const String reason = "");
	String get_connected_host() const;
	int64_t get_connected_port() const;
	WebSocketPeer::WriteMode get_write_mode() const;
	bool is_connected_to_host() const;
	void set_no_delay(const bool enabled);
	void set_write_mode(const int64_t mode);
	bool was_string_packet() const;

};

}

#endif