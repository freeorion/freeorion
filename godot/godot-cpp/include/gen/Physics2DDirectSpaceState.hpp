#ifndef GODOT_CPP_PHYSICS2DDIRECTSPACESTATE_HPP
#define GODOT_CPP_PHYSICS2DDIRECTSPACESTATE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {

class Physics2DShapeQueryParameters;

class Physics2DDirectSpaceState : public Object {
	struct ___method_bindings {
		godot_method_bind *mb_cast_motion;
		godot_method_bind *mb_collide_shape;
		godot_method_bind *mb_get_rest_info;
		godot_method_bind *mb_intersect_point;
		godot_method_bind *mb_intersect_point_on_canvas;
		godot_method_bind *mb_intersect_ray;
		godot_method_bind *mb_intersect_shape;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Physics2DDirectSpaceState"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	Array cast_motion(const Ref<Physics2DShapeQueryParameters> shape);
	Array collide_shape(const Ref<Physics2DShapeQueryParameters> shape, const int64_t max_results = 32);
	Dictionary get_rest_info(const Ref<Physics2DShapeQueryParameters> shape);
	Array intersect_point(const Vector2 point, const int64_t max_results = 32, const Array exclude = Array(), const int64_t collision_layer = 2147483647, const bool collide_with_bodies = true, const bool collide_with_areas = false);
	Array intersect_point_on_canvas(const Vector2 point, const int64_t canvas_instance_id, const int64_t max_results = 32, const Array exclude = Array(), const int64_t collision_layer = 2147483647, const bool collide_with_bodies = true, const bool collide_with_areas = false);
	Dictionary intersect_ray(const Vector2 from, const Vector2 to, const Array exclude = Array(), const int64_t collision_layer = 2147483647, const bool collide_with_bodies = true, const bool collide_with_areas = false);
	Array intersect_shape(const Ref<Physics2DShapeQueryParameters> shape, const int64_t max_results = 32);

};

}

#endif