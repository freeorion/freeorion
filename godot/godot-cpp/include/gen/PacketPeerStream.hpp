#ifndef GODOT_CPP_PACKETPEERSTREAM_HPP
#define GODOT_CPP_PACKETPEERSTREAM_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "PacketPeer.hpp"
namespace godot {

class StreamPeer;

class PacketPeerStream : public PacketPeer {
	struct ___method_bindings {
		godot_method_bind *mb_get_input_buffer_max_size;
		godot_method_bind *mb_get_output_buffer_max_size;
		godot_method_bind *mb_get_stream_peer;
		godot_method_bind *mb_set_input_buffer_max_size;
		godot_method_bind *mb_set_output_buffer_max_size;
		godot_method_bind *mb_set_stream_peer;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PacketPeerStream"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static PacketPeerStream *_new();

	// methods
	int64_t get_input_buffer_max_size() const;
	int64_t get_output_buffer_max_size() const;
	Ref<StreamPeer> get_stream_peer() const;
	void set_input_buffer_max_size(const int64_t max_size_bytes);
	void set_output_buffer_max_size(const int64_t max_size_bytes);
	void set_stream_peer(const Ref<StreamPeer> peer);

};

}

#endif