#include "CollisionObject2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "InputEvent.hpp"
#include "Shape2D.hpp"


namespace godot {


CollisionObject2D::___method_bindings CollisionObject2D::___mb = {};

void CollisionObject2D::___init_method_bindings() {
	___mb.mb__input_event = godot::api->godot_method_bind_get_method("CollisionObject2D", "_input_event");
	___mb.mb_create_shape_owner = godot::api->godot_method_bind_get_method("CollisionObject2D", "create_shape_owner");
	___mb.mb_get_rid = godot::api->godot_method_bind_get_method("CollisionObject2D", "get_rid");
	___mb.mb_get_shape_owner_one_way_collision_margin = godot::api->godot_method_bind_get_method("CollisionObject2D", "get_shape_owner_one_way_collision_margin");
	___mb.mb_get_shape_owners = godot::api->godot_method_bind_get_method("CollisionObject2D", "get_shape_owners");
	___mb.mb_is_pickable = godot::api->godot_method_bind_get_method("CollisionObject2D", "is_pickable");
	___mb.mb_is_shape_owner_disabled = godot::api->godot_method_bind_get_method("CollisionObject2D", "is_shape_owner_disabled");
	___mb.mb_is_shape_owner_one_way_collision_enabled = godot::api->godot_method_bind_get_method("CollisionObject2D", "is_shape_owner_one_way_collision_enabled");
	___mb.mb_remove_shape_owner = godot::api->godot_method_bind_get_method("CollisionObject2D", "remove_shape_owner");
	___mb.mb_set_pickable = godot::api->godot_method_bind_get_method("CollisionObject2D", "set_pickable");
	___mb.mb_shape_find_owner = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_find_owner");
	___mb.mb_shape_owner_add_shape = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_add_shape");
	___mb.mb_shape_owner_clear_shapes = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_clear_shapes");
	___mb.mb_shape_owner_get_owner = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_get_owner");
	___mb.mb_shape_owner_get_shape = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_get_shape");
	___mb.mb_shape_owner_get_shape_count = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_get_shape_count");
	___mb.mb_shape_owner_get_shape_index = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_get_shape_index");
	___mb.mb_shape_owner_get_transform = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_get_transform");
	___mb.mb_shape_owner_remove_shape = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_remove_shape");
	___mb.mb_shape_owner_set_disabled = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_set_disabled");
	___mb.mb_shape_owner_set_one_way_collision = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_set_one_way_collision");
	___mb.mb_shape_owner_set_one_way_collision_margin = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_set_one_way_collision_margin");
	___mb.mb_shape_owner_set_transform = godot::api->godot_method_bind_get_method("CollisionObject2D", "shape_owner_set_transform");
}

void CollisionObject2D::_input_event(const Object *viewport, const Ref<InputEvent> event, const int64_t shape_idx) {
	___godot_icall_void_Object_Object_int(___mb.mb__input_event, (const Object *) this, viewport, event.ptr(), shape_idx);
}

int64_t CollisionObject2D::create_shape_owner(const Object *owner) {
	return ___godot_icall_int_Object(___mb.mb_create_shape_owner, (const Object *) this, owner);
}

RID CollisionObject2D::get_rid() const {
	return ___godot_icall_RID(___mb.mb_get_rid, (const Object *) this);
}

real_t CollisionObject2D::get_shape_owner_one_way_collision_margin(const int64_t owner_id) const {
	return ___godot_icall_float_int(___mb.mb_get_shape_owner_one_way_collision_margin, (const Object *) this, owner_id);
}

Array CollisionObject2D::get_shape_owners() {
	return ___godot_icall_Array(___mb.mb_get_shape_owners, (const Object *) this);
}

bool CollisionObject2D::is_pickable() const {
	return ___godot_icall_bool(___mb.mb_is_pickable, (const Object *) this);
}

bool CollisionObject2D::is_shape_owner_disabled(const int64_t owner_id) const {
	return ___godot_icall_bool_int(___mb.mb_is_shape_owner_disabled, (const Object *) this, owner_id);
}

bool CollisionObject2D::is_shape_owner_one_way_collision_enabled(const int64_t owner_id) const {
	return ___godot_icall_bool_int(___mb.mb_is_shape_owner_one_way_collision_enabled, (const Object *) this, owner_id);
}

void CollisionObject2D::remove_shape_owner(const int64_t owner_id) {
	___godot_icall_void_int(___mb.mb_remove_shape_owner, (const Object *) this, owner_id);
}

void CollisionObject2D::set_pickable(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_pickable, (const Object *) this, enabled);
}

int64_t CollisionObject2D::shape_find_owner(const int64_t shape_index) const {
	return ___godot_icall_int_int(___mb.mb_shape_find_owner, (const Object *) this, shape_index);
}

void CollisionObject2D::shape_owner_add_shape(const int64_t owner_id, const Ref<Shape2D> shape) {
	___godot_icall_void_int_Object(___mb.mb_shape_owner_add_shape, (const Object *) this, owner_id, shape.ptr());
}

void CollisionObject2D::shape_owner_clear_shapes(const int64_t owner_id) {
	___godot_icall_void_int(___mb.mb_shape_owner_clear_shapes, (const Object *) this, owner_id);
}

Object *CollisionObject2D::shape_owner_get_owner(const int64_t owner_id) const {
	return (Object *) ___godot_icall_Object_int(___mb.mb_shape_owner_get_owner, (const Object *) this, owner_id);
}

Ref<Shape2D> CollisionObject2D::shape_owner_get_shape(const int64_t owner_id, const int64_t shape_id) const {
	return Ref<Shape2D>::__internal_constructor(___godot_icall_Object_int_int(___mb.mb_shape_owner_get_shape, (const Object *) this, owner_id, shape_id));
}

int64_t CollisionObject2D::shape_owner_get_shape_count(const int64_t owner_id) const {
	return ___godot_icall_int_int(___mb.mb_shape_owner_get_shape_count, (const Object *) this, owner_id);
}

int64_t CollisionObject2D::shape_owner_get_shape_index(const int64_t owner_id, const int64_t shape_id) const {
	return ___godot_icall_int_int_int(___mb.mb_shape_owner_get_shape_index, (const Object *) this, owner_id, shape_id);
}

Transform2D CollisionObject2D::shape_owner_get_transform(const int64_t owner_id) const {
	return ___godot_icall_Transform2D_int(___mb.mb_shape_owner_get_transform, (const Object *) this, owner_id);
}

void CollisionObject2D::shape_owner_remove_shape(const int64_t owner_id, const int64_t shape_id) {
	___godot_icall_void_int_int(___mb.mb_shape_owner_remove_shape, (const Object *) this, owner_id, shape_id);
}

void CollisionObject2D::shape_owner_set_disabled(const int64_t owner_id, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_shape_owner_set_disabled, (const Object *) this, owner_id, disabled);
}

void CollisionObject2D::shape_owner_set_one_way_collision(const int64_t owner_id, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_shape_owner_set_one_way_collision, (const Object *) this, owner_id, enable);
}

void CollisionObject2D::shape_owner_set_one_way_collision_margin(const int64_t owner_id, const real_t margin) {
	___godot_icall_void_int_float(___mb.mb_shape_owner_set_one_way_collision_margin, (const Object *) this, owner_id, margin);
}

void CollisionObject2D::shape_owner_set_transform(const int64_t owner_id, const Transform2D transform) {
	___godot_icall_void_int_Transform2D(___mb.mb_shape_owner_set_transform, (const Object *) this, owner_id, transform);
}

}