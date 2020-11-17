#ifndef GODOT_CPP_PANORAMASKY_HPP
#define GODOT_CPP_PANORAMASKY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Sky.hpp"
namespace godot {

class Texture;

class PanoramaSky : public Sky {
	struct ___method_bindings {
		godot_method_bind *mb_get_panorama;
		godot_method_bind *mb_set_panorama;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PanoramaSky"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static PanoramaSky *_new();

	// methods
	Ref<Texture> get_panorama() const;
	void set_panorama(const Ref<Texture> texture);

};

}

#endif