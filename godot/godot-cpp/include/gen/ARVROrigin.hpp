#ifndef GODOT_CPP_ARVRORIGIN_HPP
#define GODOT_CPP_ARVRORIGIN_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {


class ARVROrigin : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_get_world_scale;
		godot_method_bind *mb_set_world_scale;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ARVROrigin"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ARVROrigin *_new();

	// methods
	real_t get_world_scale() const;
	void set_world_scale(const real_t world_scale);

};

}

#endif