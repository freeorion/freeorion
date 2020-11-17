#ifndef GODOT_CPP_STREAMPEER_HPP
#define GODOT_CPP_STREAMPEER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class StreamPeer : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_16;
		godot_method_bind *mb_get_32;
		godot_method_bind *mb_get_64;
		godot_method_bind *mb_get_8;
		godot_method_bind *mb_get_available_bytes;
		godot_method_bind *mb_get_data;
		godot_method_bind *mb_get_double;
		godot_method_bind *mb_get_float;
		godot_method_bind *mb_get_partial_data;
		godot_method_bind *mb_get_string;
		godot_method_bind *mb_get_u16;
		godot_method_bind *mb_get_u32;
		godot_method_bind *mb_get_u64;
		godot_method_bind *mb_get_u8;
		godot_method_bind *mb_get_utf8_string;
		godot_method_bind *mb_get_var;
		godot_method_bind *mb_is_big_endian_enabled;
		godot_method_bind *mb_put_16;
		godot_method_bind *mb_put_32;
		godot_method_bind *mb_put_64;
		godot_method_bind *mb_put_8;
		godot_method_bind *mb_put_data;
		godot_method_bind *mb_put_double;
		godot_method_bind *mb_put_float;
		godot_method_bind *mb_put_partial_data;
		godot_method_bind *mb_put_string;
		godot_method_bind *mb_put_u16;
		godot_method_bind *mb_put_u32;
		godot_method_bind *mb_put_u64;
		godot_method_bind *mb_put_u8;
		godot_method_bind *mb_put_utf8_string;
		godot_method_bind *mb_put_var;
		godot_method_bind *mb_set_big_endian;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "StreamPeer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	int64_t get_16();
	int64_t get_32();
	int64_t get_64();
	int64_t get_8();
	int64_t get_available_bytes() const;
	Array get_data(const int64_t bytes);
	real_t get_double();
	real_t get_float();
	Array get_partial_data(const int64_t bytes);
	String get_string(const int64_t bytes = -1);
	int64_t get_u16();
	int64_t get_u32();
	int64_t get_u64();
	int64_t get_u8();
	String get_utf8_string(const int64_t bytes = -1);
	Variant get_var(const bool allow_objects = false);
	bool is_big_endian_enabled() const;
	void put_16(const int64_t value);
	void put_32(const int64_t value);
	void put_64(const int64_t value);
	void put_8(const int64_t value);
	Error put_data(const PoolByteArray data);
	void put_double(const real_t value);
	void put_float(const real_t value);
	Array put_partial_data(const PoolByteArray data);
	void put_string(const String value);
	void put_u16(const int64_t value);
	void put_u32(const int64_t value);
	void put_u64(const int64_t value);
	void put_u8(const int64_t value);
	void put_utf8_string(const String value);
	void put_var(const Variant value, const bool full_objects = false);
	void set_big_endian(const bool enable);

};

}

#endif