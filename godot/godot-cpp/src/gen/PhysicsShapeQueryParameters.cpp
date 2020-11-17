#include "PhysicsShapeQueryParameters.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"


namespace godot {


PhysicsShapeQueryParameters::___method_bindings PhysicsShapeQueryParameters::___mb = {};

void PhysicsShapeQueryParameters::___init_method_bindings() {
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "get_collision_mask");
	___mb.mb_get_exclude = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "get_exclude");
	___mb.mb_get_margin = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "get_margin");
	___mb.mb_get_shape_rid = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "get_shape_rid");
	___mb.mb_get_transform = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "get_transform");
	___mb.mb_is_collide_with_areas_enabled = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "is_collide_with_areas_enabled");
	___mb.mb_is_collide_with_bodies_enabled = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "is_collide_with_bodies_enabled");
	___mb.mb_set_collide_with_areas = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "set_collide_with_areas");
	___mb.mb_set_collide_with_bodies = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "set_collide_with_bodies");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "set_collision_mask");
	___mb.mb_set_exclude = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "set_exclude");
	___mb.mb_set_margin = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "set_margin");
	___mb.mb_set_shape = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "set_shape");
	___mb.mb_set_shape_rid = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "set_shape_rid");
	___mb.mb_set_transform = godot::api->godot_method_bind_get_method("PhysicsShapeQueryParameters", "set_transform");
}

PhysicsShapeQueryParameters *PhysicsShapeQueryParameters::_new()
{
	return (PhysicsShapeQueryParameters *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PhysicsShapeQueryParameters")());
}
int64_t PhysicsShapeQueryParameters::get_collision_mask() const {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

Array PhysicsShapeQueryParameters::get_exclude() const {
	return ___godot_icall_Array(___mb.mb_get_exclude, (const Object *) this);
}

real_t PhysicsShapeQueryParameters::get_margin() const {
	return ___godot_icall_float(___mb.mb_get_margin, (const Object *) this);
}

RID PhysicsShapeQueryParameters::get_shape_rid() const {
	return ___godot_icall_RID(___mb.mb_get_shape_rid, (const Object *) this);
}

Transform PhysicsShapeQueryParameters::get_transform() const {
	return ___godot_icall_Transform(___mb.mb_get_transform, (const Object *) this);
}

bool PhysicsShapeQueryParameters::is_collide_with_areas_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_collide_with_areas_enabled, (const Object *) this);
}

bool PhysicsShapeQueryParameters::is_collide_with_bodies_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_collide_with_bodies_enabled, (const Object *) this);
}

void PhysicsShapeQueryParameters::set_collide_with_areas(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_collide_with_areas, (const Object *) this, enable);
}

void PhysicsShapeQueryParameters::set_collide_with_bodies(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_collide_with_bodies, (const Object *) this, enable);
}

void PhysicsShapeQueryParameters::set_collision_mask(const int64_t collision_mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, collision_mask);
}

void PhysicsShapeQueryParameters::set_exclude(const Array exclude) {
	___godot_icall_void_Array(___mb.mb_set_exclude, (const Object *) this, exclude);
}

void PhysicsShapeQueryParameters::set_margin(const real_t margin) {
	___godot_icall_void_float(___mb.mb_set_margin, (const Object *) this, margin);
}

void PhysicsShapeQueryParameters::set_shape(const Ref<Resource> shape) {
	___godot_icall_void_Object(___mb.mb_set_shape, (const Object *) this, shape.ptr());
}

void PhysicsShapeQueryParameters::set_shape_rid(const RID shape) {
	___godot_icall_void_RID(___mb.mb_set_shape_rid, (const Object *) this, shape);
}

void PhysicsShapeQueryParameters::set_transform(const Transform transform) {
	___godot_icall_void_Transform(___mb.mb_set_transform, (const Object *) this, transform);
}

}