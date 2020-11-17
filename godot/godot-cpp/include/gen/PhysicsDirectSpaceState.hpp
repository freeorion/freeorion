#ifndef GODOT_CPP_PHYSICSDIRECTSPACESTATE_HPP
#define GODOT_CPP_PHYSICSDIRECTSPACESTATE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class PhysicsShapeQueryParameters;

class PhysicsDirectSpaceState : public Object {
	struct ___method_bindings {
		godot_method_bind *mb_cast_motion;
		godot_method_bind *mb_collide_shape;
		godot_method_bind *mb_get_rest_info;
		godot_method_bind *mb_intersect_ray;
		godot_method_bind *mb_intersect_shape;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PhysicsDirectSpaceState"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	Array cast_motion(const Ref<PhysicsShapeQueryParameters> shape, const Vector3 motion);
	Array collide_shape(const Ref<PhysicsShapeQueryParameters> shape, const int64_t max_results = 32);
	Dictionary get_rest_info(const Ref<PhysicsShapeQueryParameters> shape);
	Dictionary intersect_ray(const Vector3 from, const Vector3 to, const Array exclude = Array(), const int64_t collision_mask = 2147483647, const bool collide_with_bodies = true, const bool collide_with_areas = false);
	Array intersect_shape(const Ref<PhysicsShapeQueryParameters> shape, const int64_t max_results = 32);

};

}

#endif