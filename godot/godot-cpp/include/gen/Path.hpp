#ifndef GODOT_CPP_PATH_HPP
#define GODOT_CPP_PATH_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {

class Curve3D;

class Path : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb__curve_changed;
		godot_method_bind *mb_get_curve;
		godot_method_bind *mb_set_curve;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Path"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Path *_new();

	// methods
	void _curve_changed();
	Ref<Curve3D> get_curve() const;
	void set_curve(const Ref<Curve3D> curve);

};

}

#endif