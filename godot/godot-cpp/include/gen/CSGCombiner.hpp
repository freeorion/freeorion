#ifndef GODOT_CPP_CSGCOMBINER_HPP
#define GODOT_CPP_CSGCOMBINER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "CSGShape.hpp"
namespace godot {


class CSGCombiner : public CSGShape {
	struct ___method_bindings {
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CSGCombiner"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CSGCombiner *_new();

	// methods

};

}

#endif