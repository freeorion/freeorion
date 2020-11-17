#include "Physics2DTestMotionResult.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


Physics2DTestMotionResult::___method_bindings Physics2DTestMotionResult::___mb = {};

void Physics2DTestMotionResult::___init_method_bindings() {
	___mb.mb_get_collider = godot::api->godot_method_bind_get_method("Physics2DTestMotionResult", "get_collider");
	___mb.mb_get_collider_id = godot::api->godot_method_bind_get_method("Physics2DTestMotionResult", "get_collider_id");
	___mb.mb_get_collider_rid = godot::api->godot_method_bind_get_method("Physics2DTestMotionResult", "get_collider_rid");
	___mb.mb_get_collider_shape = godot::api->godot_method_bind_get_method("Physics2DTestMotionResult", "get_collider_shape");
	___mb.mb_get_collider_velocity = godot::api->godot_method_bind_get_method("Physics2DTestMotionResult", "get_collider_velocity");
	___mb.mb_get_collision_normal = godot::api->godot_method_bind_get_method("Physics2DTestMotionResult", "get_collision_normal");
	___mb.mb_get_collision_point = godot::api->godot_method_bind_get_method("Physics2DTestMotionResult", "get_collision_point");
	___mb.mb_get_motion = godot::api->godot_method_bind_get_method("Physics2DTestMotionResult", "get_motion");
	___mb.mb_get_motion_remainder = godot::api->godot_method_bind_get_method("Physics2DTestMotionResult", "get_motion_remainder");
}

Physics2DTestMotionResult *Physics2DTestMotionResult::_new()
{
	return (Physics2DTestMotionResult *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Physics2DTestMotionResult")());
}
Object *Physics2DTestMotionResult::get_collider() const {
	return (Object *) ___godot_icall_Object(___mb.mb_get_collider, (const Object *) this);
}

int64_t Physics2DTestMotionResult::get_collider_id() const {
	return ___godot_icall_int(___mb.mb_get_collider_id, (const Object *) this);
}

RID Physics2DTestMotionResult::get_collider_rid() const {
	return ___godot_icall_RID(___mb.mb_get_collider_rid, (const Object *) this);
}

int64_t Physics2DTestMotionResult::get_collider_shape() const {
	return ___godot_icall_int(___mb.mb_get_collider_shape, (const Object *) this);
}

Vector2 Physics2DTestMotionResult::get_collider_velocity() const {
	return ___godot_icall_Vector2(___mb.mb_get_collider_velocity, (const Object *) this);
}

Vector2 Physics2DTestMotionResult::get_collision_normal() const {
	return ___godot_icall_Vector2(___mb.mb_get_collision_normal, (const Object *) this);
}

Vector2 Physics2DTestMotionResult::get_collision_point() const {
	return ___godot_icall_Vector2(___mb.mb_get_collision_point, (const Object *) this);
}

Vector2 Physics2DTestMotionResult::get_motion() const {
	return ___godot_icall_Vector2(___mb.mb_get_motion, (const Object *) this);
}

Vector2 Physics2DTestMotionResult::get_motion_remainder() const {
	return ___godot_icall_Vector2(___mb.mb_get_motion_remainder, (const Object *) this);
}

}