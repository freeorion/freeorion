#ifndef GODOT_CPP_SKY_HPP
#define GODOT_CPP_SKY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Sky.hpp"

#include "Resource.hpp"
namespace godot {


class Sky : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_radiance_size;
		godot_method_bind *mb_set_radiance_size;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Sky"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum RadianceSize {
		RADIANCE_SIZE_32 = 0,
		RADIANCE_SIZE_64 = 1,
		RADIANCE_SIZE_128 = 2,
		RADIANCE_SIZE_256 = 3,
		RADIANCE_SIZE_512 = 4,
		RADIANCE_SIZE_1024 = 5,
		RADIANCE_SIZE_2048 = 6,
		RADIANCE_SIZE_MAX = 7,
	};

	// constants

	// methods
	Sky::RadianceSize get_radiance_size() const;
	void set_radiance_size(const int64_t size);

};

}

#endif