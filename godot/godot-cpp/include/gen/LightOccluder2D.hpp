#ifndef GODOT_CPP_LIGHTOCCLUDER2D_HPP
#define GODOT_CPP_LIGHTOCCLUDER2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {

class OccluderPolygon2D;

class LightOccluder2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__poly_changed;
		godot_method_bind *mb_get_occluder_light_mask;
		godot_method_bind *mb_get_occluder_polygon;
		godot_method_bind *mb_set_occluder_light_mask;
		godot_method_bind *mb_set_occluder_polygon;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "LightOccluder2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static LightOccluder2D *_new();

	// methods
	void _poly_changed();
	int64_t get_occluder_light_mask() const;
	Ref<OccluderPolygon2D> get_occluder_polygon() const;
	void set_occluder_light_mask(const int64_t mask);
	void set_occluder_polygon(const Ref<OccluderPolygon2D> polygon);

};

}

#endif