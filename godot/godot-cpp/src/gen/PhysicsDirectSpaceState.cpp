#include "PhysicsDirectSpaceState.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "PhysicsShapeQueryParameters.hpp"


namespace godot {


PhysicsDirectSpaceState::___method_bindings PhysicsDirectSpaceState::___mb = {};

void PhysicsDirectSpaceState::___init_method_bindings() {
	___mb.mb_cast_motion = godot::api->godot_method_bind_get_method("PhysicsDirectSpaceState", "cast_motion");
	___mb.mb_collide_shape = godot::api->godot_method_bind_get_method("PhysicsDirectSpaceState", "collide_shape");
	___mb.mb_get_rest_info = godot::api->godot_method_bind_get_method("PhysicsDirectSpaceState", "get_rest_info");
	___mb.mb_intersect_ray = godot::api->godot_method_bind_get_method("PhysicsDirectSpaceState", "intersect_ray");
	___mb.mb_intersect_shape = godot::api->godot_method_bind_get_method("PhysicsDirectSpaceState", "intersect_shape");
}

Array PhysicsDirectSpaceState::cast_motion(const Ref<PhysicsShapeQueryParameters> shape, const Vector3 motion) {
	return ___godot_icall_Array_Object_Vector3(___mb.mb_cast_motion, (const Object *) this, shape.ptr(), motion);
}

Array PhysicsDirectSpaceState::collide_shape(const Ref<PhysicsShapeQueryParameters> shape, const int64_t max_results) {
	return ___godot_icall_Array_Object_int(___mb.mb_collide_shape, (const Object *) this, shape.ptr(), max_results);
}

Dictionary PhysicsDirectSpaceState::get_rest_info(const Ref<PhysicsShapeQueryParameters> shape) {
	return ___godot_icall_Dictionary_Object(___mb.mb_get_rest_info, (const Object *) this, shape.ptr());
}

Dictionary PhysicsDirectSpaceState::intersect_ray(const Vector3 from, const Vector3 to, const Array exclude, const int64_t collision_mask, const bool collide_with_bodies, const bool collide_with_areas) {
	return ___godot_icall_Dictionary_Vector3_Vector3_Array_int_bool_bool(___mb.mb_intersect_ray, (const Object *) this, from, to, exclude, collision_mask, collide_with_bodies, collide_with_areas);
}

Array PhysicsDirectSpaceState::intersect_shape(const Ref<PhysicsShapeQueryParameters> shape, const int64_t max_results) {
	return ___godot_icall_Array_Object_int(___mb.mb_intersect_shape, (const Object *) this, shape.ptr(), max_results);
}

}