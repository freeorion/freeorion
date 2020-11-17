#ifndef GODOT_CPP_PACKETPEER_HPP
#define GODOT_CPP_PACKETPEER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class PacketPeer : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_available_packet_count;
		godot_method_bind *mb_get_encode_buffer_max_size;
		godot_method_bind *mb_get_packet;
		godot_method_bind *mb_get_packet_error;
		godot_method_bind *mb_get_var;
		godot_method_bind *mb_is_object_decoding_allowed;
		godot_method_bind *mb_put_packet;
		godot_method_bind *mb_put_var;
		godot_method_bind *mb_set_allow_object_decoding;
		godot_method_bind *mb_set_encode_buffer_max_size;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PacketPeer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	int64_t get_available_packet_count() const;
	int64_t get_encode_buffer_max_size() const;
	PoolByteArray get_packet();
	Error get_packet_error() const;
	Variant get_var(const bool allow_objects = false);
	bool is_object_decoding_allowed() const;
	Error put_packet(const PoolByteArray buffer);
	Error put_var(const Variant var, const bool full_objects = false);
	void set_allow_object_decoding(const bool enable);
	void set_encode_buffer_max_size(const int64_t max_size);

};

}

#endif