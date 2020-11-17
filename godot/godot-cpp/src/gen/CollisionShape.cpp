#include "CollisionShape.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Shape.hpp"
#include "Resource.hpp"


namespace godot {


CollisionShape::___method_bindings CollisionShape::___mb = {};

void CollisionShape::___init_method_bindings() {
	___mb.mb__shape_changed = godot::api->godot_method_bind_get_method("CollisionShape", "_shape_changed");
	___mb.mb__update_debug_shape = godot::api->godot_method_bind_get_method("CollisionShape", "_update_debug_shape");
	___mb.mb_get_shape = godot::api->godot_method_bind_get_method("CollisionShape", "get_shape");
	___mb.mb_is_disabled = godot::api->godot_method_bind_get_method("CollisionShape", "is_disabled");
	___mb.mb_make_convex_from_brothers = godot::api->godot_method_bind_get_method("CollisionShape", "make_convex_from_brothers");
	___mb.mb_resource_changed = godot::api->godot_method_bind_get_method("CollisionShape", "resource_changed");
	___mb.mb_set_disabled = godot::api->godot_method_bind_get_method("CollisionShape", "set_disabled");
	___mb.mb_set_shape = godot::api->godot_method_bind_get_method("CollisionShape", "set_shape");
}

CollisionShape *CollisionShape::_new()
{
	return (CollisionShape *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CollisionShape")());
}
void CollisionShape::_shape_changed() {
	___godot_icall_void(___mb.mb__shape_changed, (const Object *) this);
}

void CollisionShape::_update_debug_shape() {
	___godot_icall_void(___mb.mb__update_debug_shape, (const Object *) this);
}

Ref<Shape> CollisionShape::get_shape() const {
	return Ref<Shape>::__internal_constructor(___godot_icall_Object(___mb.mb_get_shape, (const Object *) this));
}

bool CollisionShape::is_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_disabled, (const Object *) this);
}

void CollisionShape::make_convex_from_brothers() {
	___godot_icall_void(___mb.mb_make_convex_from_brothers, (const Object *) this);
}

void CollisionShape::resource_changed(const Ref<Resource> resource) {
	___godot_icall_void_Object(___mb.mb_resource_changed, (const Object *) this, resource.ptr());
}

void CollisionShape::set_disabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_disabled, (const Object *) this, enable);
}

void CollisionShape::set_shape(const Ref<Shape> shape) {
	___godot_icall_void_Object(___mb.mb_set_shape, (const Object *) this, shape.ptr());
}

}