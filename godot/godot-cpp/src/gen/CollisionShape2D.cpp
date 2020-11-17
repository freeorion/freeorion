#include "CollisionShape2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Shape2D.hpp"


namespace godot {


CollisionShape2D::___method_bindings CollisionShape2D::___mb = {};

void CollisionShape2D::___init_method_bindings() {
	___mb.mb__shape_changed = godot::api->godot_method_bind_get_method("CollisionShape2D", "_shape_changed");
	___mb.mb_get_one_way_collision_margin = godot::api->godot_method_bind_get_method("CollisionShape2D", "get_one_way_collision_margin");
	___mb.mb_get_shape = godot::api->godot_method_bind_get_method("CollisionShape2D", "get_shape");
	___mb.mb_is_disabled = godot::api->godot_method_bind_get_method("CollisionShape2D", "is_disabled");
	___mb.mb_is_one_way_collision_enabled = godot::api->godot_method_bind_get_method("CollisionShape2D", "is_one_way_collision_enabled");
	___mb.mb_set_disabled = godot::api->godot_method_bind_get_method("CollisionShape2D", "set_disabled");
	___mb.mb_set_one_way_collision = godot::api->godot_method_bind_get_method("CollisionShape2D", "set_one_way_collision");
	___mb.mb_set_one_way_collision_margin = godot::api->godot_method_bind_get_method("CollisionShape2D", "set_one_way_collision_margin");
	___mb.mb_set_shape = godot::api->godot_method_bind_get_method("CollisionShape2D", "set_shape");
}

CollisionShape2D *CollisionShape2D::_new()
{
	return (CollisionShape2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CollisionShape2D")());
}
void CollisionShape2D::_shape_changed() {
	___godot_icall_void(___mb.mb__shape_changed, (const Object *) this);
}

real_t CollisionShape2D::get_one_way_collision_margin() const {
	return ___godot_icall_float(___mb.mb_get_one_way_collision_margin, (const Object *) this);
}

Ref<Shape2D> CollisionShape2D::get_shape() const {
	return Ref<Shape2D>::__internal_constructor(___godot_icall_Object(___mb.mb_get_shape, (const Object *) this));
}

bool CollisionShape2D::is_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_disabled, (const Object *) this);
}

bool CollisionShape2D::is_one_way_collision_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_one_way_collision_enabled, (const Object *) this);
}

void CollisionShape2D::set_disabled(const bool disabled) {
	___godot_icall_void_bool(___mb.mb_set_disabled, (const Object *) this, disabled);
}

void CollisionShape2D::set_one_way_collision(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_one_way_collision, (const Object *) this, enabled);
}

void CollisionShape2D::set_one_way_collision_margin(const real_t margin) {
	___godot_icall_void_float(___mb.mb_set_one_way_collision_margin, (const Object *) this, margin);
}

void CollisionShape2D::set_shape(const Ref<Shape2D> shape) {
	___godot_icall_void_Object(___mb.mb_set_shape, (const Object *) this, shape.ptr());
}

}