#ifndef GODOT_CPP_STREAMPEERBUFFER_HPP
#define GODOT_CPP_STREAMPEERBUFFER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "StreamPeer.hpp"
namespace godot {

class StreamPeerBuffer;

class StreamPeerBuffer : public StreamPeer {
	struct ___method_bindings {
		godot_method_bind *mb_clear;
		godot_method_bind *mb_duplicate;
		godot_method_bind *mb_get_data_array;
		godot_method_bind *mb_get_position;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_resize;
		godot_method_bind *mb_seek;
		godot_method_bind *mb_set_data_array;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "StreamPeerBuffer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static StreamPeerBuffer *_new();

	// methods
	void clear();
	Ref<StreamPeerBuffer> duplicate() const;
	PoolByteArray get_data_array() const;
	int64_t get_position() const;
	int64_t get_size() const;
	void resize(const int64_t size);
	void seek(const int64_t position);
	void set_data_array(const PoolByteArray data);

};

}

#endif