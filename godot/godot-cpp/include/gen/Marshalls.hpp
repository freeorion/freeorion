#ifndef GODOT_CPP_MARSHALLS_HPP
#define GODOT_CPP_MARSHALLS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {


class Marshalls : public Reference {
	static Marshalls *_singleton;

	Marshalls();

	struct ___method_bindings {
		godot_method_bind *mb_base64_to_raw;
		godot_method_bind *mb_base64_to_utf8;
		godot_method_bind *mb_base64_to_variant;
		godot_method_bind *mb_raw_to_base64;
		godot_method_bind *mb_utf8_to_base64;
		godot_method_bind *mb_variant_to_base64;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline Marshalls *get_singleton()
	{
		if (!Marshalls::_singleton) {
			Marshalls::_singleton = new Marshalls;
		}
		return Marshalls::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "Marshalls"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	PoolByteArray base64_to_raw(const String base64_str);
	String base64_to_utf8(const String base64_str);
	Variant base64_to_variant(const String base64_str, const bool allow_objects = false);
	String raw_to_base64(const PoolByteArray array);
	String utf8_to_base64(const String utf8_str);
	String variant_to_base64(const Variant variant, const bool full_objects = false);

};

}

#endif