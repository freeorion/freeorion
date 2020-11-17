#include "CollisionObject.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "InputEvent.hpp"
#include "Shape.hpp"


namespace godot {


CollisionObject::___method_bindings CollisionObject::___mb = {};

void CollisionObject::___init_method_bindings() {
	___mb.mb__input_event = godot::api->godot_method_bind_get_method("CollisionObject", "_input_event");
	___mb.mb_create_shape_owner = godot::api->godot_method_bind_get_method("CollisionObject", "create_shape_owner");
	___mb.mb_get_capture_input_on_drag = godot::api->godot_method_bind_get_method("CollisionObject", "get_capture_input_on_drag");
	___mb.mb_get_rid = godot::api->godot_method_bind_get_method("CollisionObject", "get_rid");
	___mb.mb_get_shape_owners = godot::api->godot_method_bind_get_method("CollisionObject", "get_shape_owners");
	___mb.mb_is_ray_pickable = godot::api->godot_method_bind_get_method("CollisionObject", "is_ray_pickable");
	___mb.mb_is_shape_owner_disabled = godot::api->godot_method_bind_get_method("CollisionObject", "is_shape_owner_disabled");
	___mb.mb_remove_shape_owner = godot::api->godot_method_bind_get_method("CollisionObject", "remove_shape_owner");
	___mb.mb_set_capture_input_on_drag = godot::api->godot_method_bind_get_method("CollisionObject", "set_capture_input_on_drag");
	___mb.mb_set_ray_pickable = godot::api->godot_method_bind_get_method("CollisionObject", "set_ray_pickable");
	___mb.mb_shape_find_owner = godot::api->godot_method_bind_get_method("CollisionObject", "shape_find_owner");
	___mb.mb_shape_owner_add_shape = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_add_shape");
	___mb.mb_shape_owner_clear_shapes = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_clear_shapes");
	___mb.mb_shape_owner_get_owner = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_get_owner");
	___mb.mb_shape_owner_get_shape = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_get_shape");
	___mb.mb_shape_owner_get_shape_count = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_get_shape_count");
	___mb.mb_shape_owner_get_shape_index = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_get_shape_index");
	___mb.mb_shape_owner_get_transform = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_get_transform");
	___mb.mb_shape_owner_remove_shape = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_remove_shape");
	___mb.mb_shape_owner_set_disabled = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_set_disabled");
	___mb.mb_shape_owner_set_transform = godot::api->godot_method_bind_get_method("CollisionObject", "shape_owner_set_transform");
}

void CollisionObject::_input_event(const Object *camera, const Ref<InputEvent> event, const Vector3 click_position, const Vector3 click_normal, const int64_t shape_idx) {
	___godot_icall_void_Object_Object_Vector3_Vector3_int(___mb.mb__input_event, (const Object *) this, camera, event.ptr(), click_position, click_normal, shape_idx);
}

int64_t CollisionObject::create_shape_owner(const Object *owner) {
	return ___godot_icall_int_Object(___mb.mb_create_shape_owner, (const Object *) this, owner);
}

bool CollisionObject::get_capture_input_on_drag() const {
	return ___godot_icall_bool(___mb.mb_get_capture_input_on_drag, (const Object *) this);
}

RID CollisionObject::get_rid() const {
	return ___godot_icall_RID(___mb.mb_get_rid, (const Object *) this);
}

Array CollisionObject::get_shape_owners() {
	return ___godot_icall_Array(___mb.mb_get_shape_owners, (const Object *) this);
}

bool CollisionObject::is_ray_pickable() const {
	return ___godot_icall_bool(___mb.mb_is_ray_pickable, (const Object *) this);
}

bool CollisionObject::is_shape_owner_disabled(const int64_t owner_id) const {
	return ___godot_icall_bool_int(___mb.mb_is_shape_owner_disabled, (const Object *) this, owner_id);
}

void CollisionObject::remove_shape_owner(const int64_t owner_id) {
	___godot_icall_void_int(___mb.mb_remove_shape_owner, (const Object *) this, owner_id);
}

void CollisionObject::set_capture_input_on_drag(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_capture_input_on_drag, (const Object *) this, enable);
}

void CollisionObject::set_ray_pickable(const bool ray_pickable) {
	___godot_icall_void_bool(___mb.mb_set_ray_pickable, (const Object *) this, ray_pickable);
}

int64_t CollisionObject::shape_find_owner(const int64_t shape_index) const {
	return ___godot_icall_int_int(___mb.mb_shape_find_owner, (const Object *) this, shape_index);
}

void CollisionObject::shape_owner_add_shape(const int64_t owner_id, const Ref<Shape> shape) {
	___godot_icall_void_int_Object(___mb.mb_shape_owner_add_shape, (const Object *) this, owner_id, shape.ptr());
}

void CollisionObject::shape_owner_clear_shapes(const int64_t owner_id) {
	___godot_icall_void_int(___mb.mb_shape_owner_clear_shapes, (const Object *) this, owner_id);
}

Object *CollisionObject::shape_owner_get_owner(const int64_t owner_id) const {
	return (Object *) ___godot_icall_Object_int(___mb.mb_shape_owner_get_owner, (const Object *) this, owner_id);
}

Ref<Shape> CollisionObject::shape_owner_get_shape(const int64_t owner_id, const int64_t shape_id) const {
	return Ref<Shape>::__internal_constructor(___godot_icall_Object_int_int(___mb.mb_shape_owner_get_shape, (const Object *) this, owner_id, shape_id));
}

int64_t CollisionObject::shape_owner_get_shape_count(const int64_t owner_id) const {
	return ___godot_icall_int_int(___mb.mb_shape_owner_get_shape_count, (const Object *) this, owner_id);
}

int64_t CollisionObject::shape_owner_get_shape_index(const int64_t owner_id, const int64_t shape_id) const {
	return ___godot_icall_int_int_int(___mb.mb_shape_owner_get_shape_index, (const Object *) this, owner_id, shape_id);
}

Transform CollisionObject::shape_owner_get_transform(const int64_t owner_id) const {
	return ___godot_icall_Transform_int(___mb.mb_shape_owner_get_transform, (const Object *) this, owner_id);
}

void CollisionObject::shape_owner_remove_shape(const int64_t owner_id, const int64_t shape_id) {
	___godot_icall_void_int_int(___mb.mb_shape_owner_remove_shape, (const Object *) this, owner_id, shape_id);
}

void CollisionObject::shape_owner_set_disabled(const int64_t owner_id, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_shape_owner_set_disabled, (const Object *) this, owner_id, disabled);
}

void CollisionObject::shape_owner_set_transform(const int64_t owner_id, const Transform transform) {
	___godot_icall_void_int_Transform(___mb.mb_shape_owner_set_transform, (const Object *) this, owner_id, transform);
}

}