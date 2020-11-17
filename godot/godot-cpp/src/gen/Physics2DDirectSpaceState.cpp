#include "Physics2DDirectSpaceState.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Physics2DShapeQueryParameters.hpp"


namespace godot {


Physics2DDirectSpaceState::___method_bindings Physics2DDirectSpaceState::___mb = {};

void Physics2DDirectSpaceState::___init_method_bindings() {
	___mb.mb_cast_motion = godot::api->godot_method_bind_get_method("Physics2DDirectSpaceState", "cast_motion");
	___mb.mb_collide_shape = godot::api->godot_method_bind_get_method("Physics2DDirectSpaceState", "collide_shape");
	___mb.mb_get_rest_info = godot::api->godot_method_bind_get_method("Physics2DDirectSpaceState", "get_rest_info");
	___mb.mb_intersect_point = godot::api->godot_method_bind_get_method("Physics2DDirectSpaceState", "intersect_point");
	___mb.mb_intersect_point_on_canvas = godot::api->godot_method_bind_get_method("Physics2DDirectSpaceState", "intersect_point_on_canvas");
	___mb.mb_intersect_ray = godot::api->godot_method_bind_get_method("Physics2DDirectSpaceState", "intersect_ray");
	___mb.mb_intersect_shape = godot::api->godot_method_bind_get_method("Physics2DDirectSpaceState", "intersect_shape");
}

Array Physics2DDirectSpaceState::cast_motion(const Ref<Physics2DShapeQueryParameters> shape) {
	return ___godot_icall_Array_Object(___mb.mb_cast_motion, (const Object *) this, shape.ptr());
}

Array Physics2DDirectSpaceState::collide_shape(const Ref<Physics2DShapeQueryParameters> shape, const int64_t max_results) {
	return ___godot_icall_Array_Object_int(___mb.mb_collide_shape, (const Object *) this, shape.ptr(), max_results);
}

Dictionary Physics2DDirectSpaceState::get_rest_info(const Ref<Physics2DShapeQueryParameters> shape) {
	return ___godot_icall_Dictionary_Object(___mb.mb_get_rest_info, (const Object *) this, shape.ptr());
}

Array Physics2DDirectSpaceState::intersect_point(const Vector2 point, const int64_t max_results, const Array exclude, const int64_t collision_layer, const bool collide_with_bodies, const bool collide_with_areas) {
	return ___godot_icall_Array_Vector2_int_Array_int_bool_bool(___mb.mb_intersect_point, (const Object *) this, point, max_results, exclude, collision_layer, collide_with_bodies, collide_with_areas);
}

Array Physics2DDirectSpaceState::intersect_point_on_canvas(const Vector2 point, const int64_t canvas_instance_id, const int64_t max_results, const Array exclude, const int64_t collision_layer, const bool collide_with_bodies, const bool collide_with_areas) {
	return ___godot_icall_Array_Vector2_int_int_Array_int_bool_bool(___mb.mb_intersect_point_on_canvas, (const Object *) this, point, canvas_instance_id, max_results, exclude, collision_layer, collide_with_bodies, collide_with_areas);
}

Dictionary Physics2DDirectSpaceState::intersect_ray(const Vector2 from, const Vector2 to, const Array exclude, const int64_t collision_layer, const bool collide_with_bodies, const bool collide_with_areas) {
	return ___godot_icall_Dictionary_Vector2_Vector2_Array_int_bool_bool(___mb.mb_intersect_ray, (const Object *) this, from, to, exclude, collision_layer, collide_with_bodies, collide_with_areas);
}

Array Physics2DDirectSpaceState::intersect_shape(const Ref<Physics2DShapeQueryParameters> shape, const int64_t max_results) {
	return ___godot_icall_Array_Object_int(___mb.mb_intersect_shape, (const Object *) this, shape.ptr(), max_results);
}

}