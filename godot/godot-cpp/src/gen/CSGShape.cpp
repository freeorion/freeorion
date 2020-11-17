#include "CSGShape.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CSGShape::___method_bindings CSGShape::___mb = {};

void CSGShape::___init_method_bindings() {
	___mb.mb__update_shape = godot::api->godot_method_bind_get_method("CSGShape", "_update_shape");
	___mb.mb_get_collision_layer = godot::api->godot_method_bind_get_method("CSGShape", "get_collision_layer");
	___mb.mb_get_collision_layer_bit = godot::api->godot_method_bind_get_method("CSGShape", "get_collision_layer_bit");
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("CSGShape", "get_collision_mask");
	___mb.mb_get_collision_mask_bit = godot::api->godot_method_bind_get_method("CSGShape", "get_collision_mask_bit");
	___mb.mb_get_meshes = godot::api->godot_method_bind_get_method("CSGShape", "get_meshes");
	___mb.mb_get_operation = godot::api->godot_method_bind_get_method("CSGShape", "get_operation");
	___mb.mb_get_snap = godot::api->godot_method_bind_get_method("CSGShape", "get_snap");
	___mb.mb_is_calculating_tangents = godot::api->godot_method_bind_get_method("CSGShape", "is_calculating_tangents");
	___mb.mb_is_root_shape = godot::api->godot_method_bind_get_method("CSGShape", "is_root_shape");
	___mb.mb_is_using_collision = godot::api->godot_method_bind_get_method("CSGShape", "is_using_collision");
	___mb.mb_set_calculate_tangents = godot::api->godot_method_bind_get_method("CSGShape", "set_calculate_tangents");
	___mb.mb_set_collision_layer = godot::api->godot_method_bind_get_method("CSGShape", "set_collision_layer");
	___mb.mb_set_collision_layer_bit = godot::api->godot_method_bind_get_method("CSGShape", "set_collision_layer_bit");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("CSGShape", "set_collision_mask");
	___mb.mb_set_collision_mask_bit = godot::api->godot_method_bind_get_method("CSGShape", "set_collision_mask_bit");
	___mb.mb_set_operation = godot::api->godot_method_bind_get_method("CSGShape", "set_operation");
	___mb.mb_set_snap = godot::api->godot_method_bind_get_method("CSGShape", "set_snap");
	___mb.mb_set_use_collision = godot::api->godot_method_bind_get_method("CSGShape", "set_use_collision");
}

void CSGShape::_update_shape() {
	___godot_icall_void(___mb.mb__update_shape, (const Object *) this);
}

int64_t CSGShape::get_collision_layer() const {
	return ___godot_icall_int(___mb.mb_get_collision_layer, (const Object *) this);
}

bool CSGShape::get_collision_layer_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_layer_bit, (const Object *) this, bit);
}

int64_t CSGShape::get_collision_mask() const {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

bool CSGShape::get_collision_mask_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_mask_bit, (const Object *) this, bit);
}

Array CSGShape::get_meshes() const {
	return ___godot_icall_Array(___mb.mb_get_meshes, (const Object *) this);
}

CSGShape::Operation CSGShape::get_operation() const {
	return (CSGShape::Operation) ___godot_icall_int(___mb.mb_get_operation, (const Object *) this);
}

real_t CSGShape::get_snap() const {
	return ___godot_icall_float(___mb.mb_get_snap, (const Object *) this);
}

bool CSGShape::is_calculating_tangents() const {
	return ___godot_icall_bool(___mb.mb_is_calculating_tangents, (const Object *) this);
}

bool CSGShape::is_root_shape() const {
	return ___godot_icall_bool(___mb.mb_is_root_shape, (const Object *) this);
}

bool CSGShape::is_using_collision() const {
	return ___godot_icall_bool(___mb.mb_is_using_collision, (const Object *) this);
}

void CSGShape::set_calculate_tangents(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_calculate_tangents, (const Object *) this, enabled);
}

void CSGShape::set_collision_layer(const int64_t layer) {
	___godot_icall_void_int(___mb.mb_set_collision_layer, (const Object *) this, layer);
}

void CSGShape::set_collision_layer_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_layer_bit, (const Object *) this, bit, value);
}

void CSGShape::set_collision_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, mask);
}

void CSGShape::set_collision_mask_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_mask_bit, (const Object *) this, bit, value);
}

void CSGShape::set_operation(const int64_t operation) {
	___godot_icall_void_int(___mb.mb_set_operation, (const Object *) this, operation);
}

void CSGShape::set_snap(const real_t snap) {
	___godot_icall_void_float(___mb.mb_set_snap, (const Object *) this, snap);
}

void CSGShape::set_use_collision(const bool operation) {
	___godot_icall_void_bool(___mb.mb_set_use_collision, (const Object *) this, operation);
}

}