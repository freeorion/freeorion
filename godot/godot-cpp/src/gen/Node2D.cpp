#include "Node2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"


namespace godot {


Node2D::___method_bindings Node2D::___mb = {};

void Node2D::___init_method_bindings() {
	___mb.mb_apply_scale = godot::api->godot_method_bind_get_method("Node2D", "apply_scale");
	___mb.mb_get_angle_to = godot::api->godot_method_bind_get_method("Node2D", "get_angle_to");
	___mb.mb_get_global_position = godot::api->godot_method_bind_get_method("Node2D", "get_global_position");
	___mb.mb_get_global_rotation = godot::api->godot_method_bind_get_method("Node2D", "get_global_rotation");
	___mb.mb_get_global_rotation_degrees = godot::api->godot_method_bind_get_method("Node2D", "get_global_rotation_degrees");
	___mb.mb_get_global_scale = godot::api->godot_method_bind_get_method("Node2D", "get_global_scale");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("Node2D", "get_position");
	___mb.mb_get_relative_transform_to_parent = godot::api->godot_method_bind_get_method("Node2D", "get_relative_transform_to_parent");
	___mb.mb_get_rotation = godot::api->godot_method_bind_get_method("Node2D", "get_rotation");
	___mb.mb_get_rotation_degrees = godot::api->godot_method_bind_get_method("Node2D", "get_rotation_degrees");
	___mb.mb_get_scale = godot::api->godot_method_bind_get_method("Node2D", "get_scale");
	___mb.mb_get_z_index = godot::api->godot_method_bind_get_method("Node2D", "get_z_index");
	___mb.mb_global_translate = godot::api->godot_method_bind_get_method("Node2D", "global_translate");
	___mb.mb_is_z_relative = godot::api->godot_method_bind_get_method("Node2D", "is_z_relative");
	___mb.mb_look_at = godot::api->godot_method_bind_get_method("Node2D", "look_at");
	___mb.mb_move_local_x = godot::api->godot_method_bind_get_method("Node2D", "move_local_x");
	___mb.mb_move_local_y = godot::api->godot_method_bind_get_method("Node2D", "move_local_y");
	___mb.mb_rotate = godot::api->godot_method_bind_get_method("Node2D", "rotate");
	___mb.mb_set_global_position = godot::api->godot_method_bind_get_method("Node2D", "set_global_position");
	___mb.mb_set_global_rotation = godot::api->godot_method_bind_get_method("Node2D", "set_global_rotation");
	___mb.mb_set_global_rotation_degrees = godot::api->godot_method_bind_get_method("Node2D", "set_global_rotation_degrees");
	___mb.mb_set_global_scale = godot::api->godot_method_bind_get_method("Node2D", "set_global_scale");
	___mb.mb_set_global_transform = godot::api->godot_method_bind_get_method("Node2D", "set_global_transform");
	___mb.mb_set_position = godot::api->godot_method_bind_get_method("Node2D", "set_position");
	___mb.mb_set_rotation = godot::api->godot_method_bind_get_method("Node2D", "set_rotation");
	___mb.mb_set_rotation_degrees = godot::api->godot_method_bind_get_method("Node2D", "set_rotation_degrees");
	___mb.mb_set_scale = godot::api->godot_method_bind_get_method("Node2D", "set_scale");
	___mb.mb_set_transform = godot::api->godot_method_bind_get_method("Node2D", "set_transform");
	___mb.mb_set_z_as_relative = godot::api->godot_method_bind_get_method("Node2D", "set_z_as_relative");
	___mb.mb_set_z_index = godot::api->godot_method_bind_get_method("Node2D", "set_z_index");
	___mb.mb_to_global = godot::api->godot_method_bind_get_method("Node2D", "to_global");
	___mb.mb_to_local = godot::api->godot_method_bind_get_method("Node2D", "to_local");
	___mb.mb_translate = godot::api->godot_method_bind_get_method("Node2D", "translate");
}

Node2D *Node2D::_new()
{
	return (Node2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Node2D")());
}
void Node2D::apply_scale(const Vector2 ratio) {
	___godot_icall_void_Vector2(___mb.mb_apply_scale, (const Object *) this, ratio);
}

real_t Node2D::get_angle_to(const Vector2 point) const {
	return ___godot_icall_float_Vector2(___mb.mb_get_angle_to, (const Object *) this, point);
}

Vector2 Node2D::get_global_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_global_position, (const Object *) this);
}

real_t Node2D::get_global_rotation() const {
	return ___godot_icall_float(___mb.mb_get_global_rotation, (const Object *) this);
}

real_t Node2D::get_global_rotation_degrees() const {
	return ___godot_icall_float(___mb.mb_get_global_rotation_degrees, (const Object *) this);
}

Vector2 Node2D::get_global_scale() const {
	return ___godot_icall_Vector2(___mb.mb_get_global_scale, (const Object *) this);
}

Vector2 Node2D::get_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_position, (const Object *) this);
}

Transform2D Node2D::get_relative_transform_to_parent(const Node *parent) const {
	return ___godot_icall_Transform2D_Object(___mb.mb_get_relative_transform_to_parent, (const Object *) this, parent);
}

real_t Node2D::get_rotation() const {
	return ___godot_icall_float(___mb.mb_get_rotation, (const Object *) this);
}

real_t Node2D::get_rotation_degrees() const {
	return ___godot_icall_float(___mb.mb_get_rotation_degrees, (const Object *) this);
}

Vector2 Node2D::get_scale() const {
	return ___godot_icall_Vector2(___mb.mb_get_scale, (const Object *) this);
}

int64_t Node2D::get_z_index() const {
	return ___godot_icall_int(___mb.mb_get_z_index, (const Object *) this);
}

void Node2D::global_translate(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_global_translate, (const Object *) this, offset);
}

bool Node2D::is_z_relative() const {
	return ___godot_icall_bool(___mb.mb_is_z_relative, (const Object *) this);
}

void Node2D::look_at(const Vector2 point) {
	___godot_icall_void_Vector2(___mb.mb_look_at, (const Object *) this, point);
}

void Node2D::move_local_x(const real_t delta, const bool scaled) {
	___godot_icall_void_float_bool(___mb.mb_move_local_x, (const Object *) this, delta, scaled);
}

void Node2D::move_local_y(const real_t delta, const bool scaled) {
	___godot_icall_void_float_bool(___mb.mb_move_local_y, (const Object *) this, delta, scaled);
}

void Node2D::rotate(const real_t radians) {
	___godot_icall_void_float(___mb.mb_rotate, (const Object *) this, radians);
}

void Node2D::set_global_position(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_set_global_position, (const Object *) this, position);
}

void Node2D::set_global_rotation(const real_t radians) {
	___godot_icall_void_float(___mb.mb_set_global_rotation, (const Object *) this, radians);
}

void Node2D::set_global_rotation_degrees(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_global_rotation_degrees, (const Object *) this, degrees);
}

void Node2D::set_global_scale(const Vector2 scale) {
	___godot_icall_void_Vector2(___mb.mb_set_global_scale, (const Object *) this, scale);
}

void Node2D::set_global_transform(const Transform2D xform) {
	___godot_icall_void_Transform2D(___mb.mb_set_global_transform, (const Object *) this, xform);
}

void Node2D::set_position(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_set_position, (const Object *) this, position);
}

void Node2D::set_rotation(const real_t radians) {
	___godot_icall_void_float(___mb.mb_set_rotation, (const Object *) this, radians);
}

void Node2D::set_rotation_degrees(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_rotation_degrees, (const Object *) this, degrees);
}

void Node2D::set_scale(const Vector2 scale) {
	___godot_icall_void_Vector2(___mb.mb_set_scale, (const Object *) this, scale);
}

void Node2D::set_transform(const Transform2D xform) {
	___godot_icall_void_Transform2D(___mb.mb_set_transform, (const Object *) this, xform);
}

void Node2D::set_z_as_relative(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_z_as_relative, (const Object *) this, enable);
}

void Node2D::set_z_index(const int64_t z_index) {
	___godot_icall_void_int(___mb.mb_set_z_index, (const Object *) this, z_index);
}

Vector2 Node2D::to_global(const Vector2 local_point) const {
	return ___godot_icall_Vector2_Vector2(___mb.mb_to_global, (const Object *) this, local_point);
}

Vector2 Node2D::to_local(const Vector2 global_point) const {
	return ___godot_icall_Vector2_Vector2(___mb.mb_to_local, (const Object *) this, global_point);
}

void Node2D::translate(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_translate, (const Object *) this, offset);
}

}