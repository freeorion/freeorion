#include "SpatialVelocityTracker.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


SpatialVelocityTracker::___method_bindings SpatialVelocityTracker::___mb = {};

void SpatialVelocityTracker::___init_method_bindings() {
	___mb.mb_get_tracked_linear_velocity = godot::api->godot_method_bind_get_method("SpatialVelocityTracker", "get_tracked_linear_velocity");
	___mb.mb_is_tracking_physics_step = godot::api->godot_method_bind_get_method("SpatialVelocityTracker", "is_tracking_physics_step");
	___mb.mb_reset = godot::api->godot_method_bind_get_method("SpatialVelocityTracker", "reset");
	___mb.mb_set_track_physics_step = godot::api->godot_method_bind_get_method("SpatialVelocityTracker", "set_track_physics_step");
	___mb.mb_update_position = godot::api->godot_method_bind_get_method("SpatialVelocityTracker", "update_position");
}

SpatialVelocityTracker *SpatialVelocityTracker::_new()
{
	return (SpatialVelocityTracker *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SpatialVelocityTracker")());
}
Vector3 SpatialVelocityTracker::get_tracked_linear_velocity() const {
	return ___godot_icall_Vector3(___mb.mb_get_tracked_linear_velocity, (const Object *) this);
}

bool SpatialVelocityTracker::is_tracking_physics_step() const {
	return ___godot_icall_bool(___mb.mb_is_tracking_physics_step, (const Object *) this);
}

void SpatialVelocityTracker::reset(const Vector3 position) {
	___godot_icall_void_Vector3(___mb.mb_reset, (const Object *) this, position);
}

void SpatialVelocityTracker::set_track_physics_step(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_track_physics_step, (const Object *) this, enable);
}

void SpatialVelocityTracker::update_position(const Vector3 position) {
	___godot_icall_void_Vector3(___mb.mb_update_position, (const Object *) this, position);
}

}