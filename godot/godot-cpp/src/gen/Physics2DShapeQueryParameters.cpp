#include "Physics2DShapeQueryParameters.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"


namespace godot {


Physics2DShapeQueryParameters::___method_bindings Physics2DShapeQueryParameters::___mb = {};

void Physics2DShapeQueryParameters::___init_method_bindings() {
	___mb.mb_get_collision_layer = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "get_collision_layer");
	___mb.mb_get_exclude = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "get_exclude");
	___mb.mb_get_margin = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "get_margin");
	___mb.mb_get_motion = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "get_motion");
	___mb.mb_get_shape_rid = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "get_shape_rid");
	___mb.mb_get_transform = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "get_transform");
	___mb.mb_is_collide_with_areas_enabled = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "is_collide_with_areas_enabled");
	___mb.mb_is_collide_with_bodies_enabled = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "is_collide_with_bodies_enabled");
	___mb.mb_set_collide_with_areas = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "set_collide_with_areas");
	___mb.mb_set_collide_with_bodies = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "set_collide_with_bodies");
	___mb.mb_set_collision_layer = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "set_collision_layer");
	___mb.mb_set_exclude = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "set_exclude");
	___mb.mb_set_margin = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "set_margin");
	___mb.mb_set_motion = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "set_motion");
	___mb.mb_set_shape = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "set_shape");
	___mb.mb_set_shape_rid = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "set_shape_rid");
	___mb.mb_set_transform = godot::api->godot_method_bind_get_method("Physics2DShapeQueryParameters", "set_transform");
}

Physics2DShapeQueryParameters *Physics2DShapeQueryParameters::_new()
{
	return (Physics2DShapeQueryParameters *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Physics2DShapeQueryParameters")());
}
int64_t Physics2DShapeQueryParameters::get_collision_layer() const {
	return ___godot_icall_int(___mb.mb_get_collision_layer, (const Object *) this);
}

Array Physics2DShapeQueryParameters::get_exclude() const {
	return ___godot_icall_Array(___mb.mb_get_exclude, (const Object *) this);
}

real_t Physics2DShapeQueryParameters::get_margin() const {
	return ___godot_icall_float(___mb.mb_get_margin, (const Object *) this);
}

Vector2 Physics2DShapeQueryParameters::get_motion() const {
	return ___godot_icall_Vector2(___mb.mb_get_motion, (const Object *) this);
}

RID Physics2DShapeQueryParameters::get_shape_rid() const {
	return ___godot_icall_RID(___mb.mb_get_shape_rid, (const Object *) this);
}

Transform2D Physics2DShapeQueryParameters::get_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_transform, (const Object *) this);
}

bool Physics2DShapeQueryParameters::is_collide_with_areas_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_collide_with_areas_enabled, (const Object *) this);
}

bool Physics2DShapeQueryParameters::is_collide_with_bodies_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_collide_with_bodies_enabled, (const Object *) this);
}

void Physics2DShapeQueryParameters::set_collide_with_areas(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_collide_with_areas, (const Object *) this, enable);
}

void Physics2DShapeQueryParameters::set_collide_with_bodies(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_collide_with_bodies, (const Object *) this, enable);
}

void Physics2DShapeQueryParameters::set_collision_layer(const int64_t collision_layer) {
	___godot_icall_void_int(___mb.mb_set_collision_layer, (const Object *) this, collision_layer);
}

void Physics2DShapeQueryParameters::set_exclude(const Array exclude) {
	___godot_icall_void_Array(___mb.mb_set_exclude, (const Object *) this, exclude);
}

void Physics2DShapeQueryParameters::set_margin(const real_t margin) {
	___godot_icall_void_float(___mb.mb_set_margin, (const Object *) this, margin);
}

void Physics2DShapeQueryParameters::set_motion(const Vector2 motion) {
	___godot_icall_void_Vector2(___mb.mb_set_motion, (const Object *) this, motion);
}

void Physics2DShapeQueryParameters::set_shape(const Ref<Resource> shape) {
	___godot_icall_void_Object(___mb.mb_set_shape, (const Object *) this, shape.ptr());
}

void Physics2DShapeQueryParameters::set_shape_rid(const RID shape) {
	___godot_icall_void_RID(___mb.mb_set_shape_rid, (const Object *) this, shape);
}

void Physics2DShapeQueryParameters::set_transform(const Transform2D transform) {
	___godot_icall_void_Transform2D(___mb.mb_set_transform, (const Object *) this, transform);
}

}