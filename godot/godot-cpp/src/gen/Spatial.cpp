#include "Spatial.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "SpatialGizmo.hpp"
#include "Spatial.hpp"
#include "World.hpp"


namespace godot {


Spatial::___method_bindings Spatial::___mb = {};

void Spatial::___init_method_bindings() {
	___mb.mb__update_gizmo = godot::api->godot_method_bind_get_method("Spatial", "_update_gizmo");
	___mb.mb_force_update_transform = godot::api->godot_method_bind_get_method("Spatial", "force_update_transform");
	___mb.mb_get_gizmo = godot::api->godot_method_bind_get_method("Spatial", "get_gizmo");
	___mb.mb_get_global_transform = godot::api->godot_method_bind_get_method("Spatial", "get_global_transform");
	___mb.mb_get_parent_spatial = godot::api->godot_method_bind_get_method("Spatial", "get_parent_spatial");
	___mb.mb_get_rotation = godot::api->godot_method_bind_get_method("Spatial", "get_rotation");
	___mb.mb_get_rotation_degrees = godot::api->godot_method_bind_get_method("Spatial", "get_rotation_degrees");
	___mb.mb_get_scale = godot::api->godot_method_bind_get_method("Spatial", "get_scale");
	___mb.mb_get_transform = godot::api->godot_method_bind_get_method("Spatial", "get_transform");
	___mb.mb_get_translation = godot::api->godot_method_bind_get_method("Spatial", "get_translation");
	___mb.mb_get_world = godot::api->godot_method_bind_get_method("Spatial", "get_world");
	___mb.mb_global_rotate = godot::api->godot_method_bind_get_method("Spatial", "global_rotate");
	___mb.mb_global_scale = godot::api->godot_method_bind_get_method("Spatial", "global_scale");
	___mb.mb_global_translate = godot::api->godot_method_bind_get_method("Spatial", "global_translate");
	___mb.mb_hide = godot::api->godot_method_bind_get_method("Spatial", "hide");
	___mb.mb_is_local_transform_notification_enabled = godot::api->godot_method_bind_get_method("Spatial", "is_local_transform_notification_enabled");
	___mb.mb_is_scale_disabled = godot::api->godot_method_bind_get_method("Spatial", "is_scale_disabled");
	___mb.mb_is_set_as_toplevel = godot::api->godot_method_bind_get_method("Spatial", "is_set_as_toplevel");
	___mb.mb_is_transform_notification_enabled = godot::api->godot_method_bind_get_method("Spatial", "is_transform_notification_enabled");
	___mb.mb_is_visible = godot::api->godot_method_bind_get_method("Spatial", "is_visible");
	___mb.mb_is_visible_in_tree = godot::api->godot_method_bind_get_method("Spatial", "is_visible_in_tree");
	___mb.mb_look_at = godot::api->godot_method_bind_get_method("Spatial", "look_at");
	___mb.mb_look_at_from_position = godot::api->godot_method_bind_get_method("Spatial", "look_at_from_position");
	___mb.mb_orthonormalize = godot::api->godot_method_bind_get_method("Spatial", "orthonormalize");
	___mb.mb_rotate = godot::api->godot_method_bind_get_method("Spatial", "rotate");
	___mb.mb_rotate_object_local = godot::api->godot_method_bind_get_method("Spatial", "rotate_object_local");
	___mb.mb_rotate_x = godot::api->godot_method_bind_get_method("Spatial", "rotate_x");
	___mb.mb_rotate_y = godot::api->godot_method_bind_get_method("Spatial", "rotate_y");
	___mb.mb_rotate_z = godot::api->godot_method_bind_get_method("Spatial", "rotate_z");
	___mb.mb_scale_object_local = godot::api->godot_method_bind_get_method("Spatial", "scale_object_local");
	___mb.mb_set_as_toplevel = godot::api->godot_method_bind_get_method("Spatial", "set_as_toplevel");
	___mb.mb_set_disable_scale = godot::api->godot_method_bind_get_method("Spatial", "set_disable_scale");
	___mb.mb_set_gizmo = godot::api->godot_method_bind_get_method("Spatial", "set_gizmo");
	___mb.mb_set_global_transform = godot::api->godot_method_bind_get_method("Spatial", "set_global_transform");
	___mb.mb_set_identity = godot::api->godot_method_bind_get_method("Spatial", "set_identity");
	___mb.mb_set_ignore_transform_notification = godot::api->godot_method_bind_get_method("Spatial", "set_ignore_transform_notification");
	___mb.mb_set_notify_local_transform = godot::api->godot_method_bind_get_method("Spatial", "set_notify_local_transform");
	___mb.mb_set_notify_transform = godot::api->godot_method_bind_get_method("Spatial", "set_notify_transform");
	___mb.mb_set_rotation = godot::api->godot_method_bind_get_method("Spatial", "set_rotation");
	___mb.mb_set_rotation_degrees = godot::api->godot_method_bind_get_method("Spatial", "set_rotation_degrees");
	___mb.mb_set_scale = godot::api->godot_method_bind_get_method("Spatial", "set_scale");
	___mb.mb_set_transform = godot::api->godot_method_bind_get_method("Spatial", "set_transform");
	___mb.mb_set_translation = godot::api->godot_method_bind_get_method("Spatial", "set_translation");
	___mb.mb_set_visible = godot::api->godot_method_bind_get_method("Spatial", "set_visible");
	___mb.mb_show = godot::api->godot_method_bind_get_method("Spatial", "show");
	___mb.mb_to_global = godot::api->godot_method_bind_get_method("Spatial", "to_global");
	___mb.mb_to_local = godot::api->godot_method_bind_get_method("Spatial", "to_local");
	___mb.mb_translate = godot::api->godot_method_bind_get_method("Spatial", "translate");
	___mb.mb_translate_object_local = godot::api->godot_method_bind_get_method("Spatial", "translate_object_local");
	___mb.mb_update_gizmo = godot::api->godot_method_bind_get_method("Spatial", "update_gizmo");
}

Spatial *Spatial::_new()
{
	return (Spatial *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Spatial")());
}
void Spatial::_update_gizmo() {
	___godot_icall_void(___mb.mb__update_gizmo, (const Object *) this);
}

void Spatial::force_update_transform() {
	___godot_icall_void(___mb.mb_force_update_transform, (const Object *) this);
}

Ref<SpatialGizmo> Spatial::get_gizmo() const {
	return Ref<SpatialGizmo>::__internal_constructor(___godot_icall_Object(___mb.mb_get_gizmo, (const Object *) this));
}

Transform Spatial::get_global_transform() const {
	return ___godot_icall_Transform(___mb.mb_get_global_transform, (const Object *) this);
}

Spatial *Spatial::get_parent_spatial() const {
	return (Spatial *) ___godot_icall_Object(___mb.mb_get_parent_spatial, (const Object *) this);
}

Vector3 Spatial::get_rotation() const {
	return ___godot_icall_Vector3(___mb.mb_get_rotation, (const Object *) this);
}

Vector3 Spatial::get_rotation_degrees() const {
	return ___godot_icall_Vector3(___mb.mb_get_rotation_degrees, (const Object *) this);
}

Vector3 Spatial::get_scale() const {
	return ___godot_icall_Vector3(___mb.mb_get_scale, (const Object *) this);
}

Transform Spatial::get_transform() const {
	return ___godot_icall_Transform(___mb.mb_get_transform, (const Object *) this);
}

Vector3 Spatial::get_translation() const {
	return ___godot_icall_Vector3(___mb.mb_get_translation, (const Object *) this);
}

Ref<World> Spatial::get_world() const {
	return Ref<World>::__internal_constructor(___godot_icall_Object(___mb.mb_get_world, (const Object *) this));
}

void Spatial::global_rotate(const Vector3 axis, const real_t angle) {
	___godot_icall_void_Vector3_float(___mb.mb_global_rotate, (const Object *) this, axis, angle);
}

void Spatial::global_scale(const Vector3 scale) {
	___godot_icall_void_Vector3(___mb.mb_global_scale, (const Object *) this, scale);
}

void Spatial::global_translate(const Vector3 offset) {
	___godot_icall_void_Vector3(___mb.mb_global_translate, (const Object *) this, offset);
}

void Spatial::hide() {
	___godot_icall_void(___mb.mb_hide, (const Object *) this);
}

bool Spatial::is_local_transform_notification_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_local_transform_notification_enabled, (const Object *) this);
}

bool Spatial::is_scale_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_scale_disabled, (const Object *) this);
}

bool Spatial::is_set_as_toplevel() const {
	return ___godot_icall_bool(___mb.mb_is_set_as_toplevel, (const Object *) this);
}

bool Spatial::is_transform_notification_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_transform_notification_enabled, (const Object *) this);
}

bool Spatial::is_visible() const {
	return ___godot_icall_bool(___mb.mb_is_visible, (const Object *) this);
}

bool Spatial::is_visible_in_tree() const {
	return ___godot_icall_bool(___mb.mb_is_visible_in_tree, (const Object *) this);
}

void Spatial::look_at(const Vector3 target, const Vector3 up) {
	___godot_icall_void_Vector3_Vector3(___mb.mb_look_at, (const Object *) this, target, up);
}

void Spatial::look_at_from_position(const Vector3 position, const Vector3 target, const Vector3 up) {
	___godot_icall_void_Vector3_Vector3_Vector3(___mb.mb_look_at_from_position, (const Object *) this, position, target, up);
}

void Spatial::orthonormalize() {
	___godot_icall_void(___mb.mb_orthonormalize, (const Object *) this);
}

void Spatial::rotate(const Vector3 axis, const real_t angle) {
	___godot_icall_void_Vector3_float(___mb.mb_rotate, (const Object *) this, axis, angle);
}

void Spatial::rotate_object_local(const Vector3 axis, const real_t angle) {
	___godot_icall_void_Vector3_float(___mb.mb_rotate_object_local, (const Object *) this, axis, angle);
}

void Spatial::rotate_x(const real_t angle) {
	___godot_icall_void_float(___mb.mb_rotate_x, (const Object *) this, angle);
}

void Spatial::rotate_y(const real_t angle) {
	___godot_icall_void_float(___mb.mb_rotate_y, (const Object *) this, angle);
}

void Spatial::rotate_z(const real_t angle) {
	___godot_icall_void_float(___mb.mb_rotate_z, (const Object *) this, angle);
}

void Spatial::scale_object_local(const Vector3 scale) {
	___godot_icall_void_Vector3(___mb.mb_scale_object_local, (const Object *) this, scale);
}

void Spatial::set_as_toplevel(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_as_toplevel, (const Object *) this, enable);
}

void Spatial::set_disable_scale(const bool disable) {
	___godot_icall_void_bool(___mb.mb_set_disable_scale, (const Object *) this, disable);
}

void Spatial::set_gizmo(const Ref<SpatialGizmo> gizmo) {
	___godot_icall_void_Object(___mb.mb_set_gizmo, (const Object *) this, gizmo.ptr());
}

void Spatial::set_global_transform(const Transform global) {
	___godot_icall_void_Transform(___mb.mb_set_global_transform, (const Object *) this, global);
}

void Spatial::set_identity() {
	___godot_icall_void(___mb.mb_set_identity, (const Object *) this);
}

void Spatial::set_ignore_transform_notification(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_ignore_transform_notification, (const Object *) this, enabled);
}

void Spatial::set_notify_local_transform(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_notify_local_transform, (const Object *) this, enable);
}

void Spatial::set_notify_transform(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_notify_transform, (const Object *) this, enable);
}

void Spatial::set_rotation(const Vector3 euler) {
	___godot_icall_void_Vector3(___mb.mb_set_rotation, (const Object *) this, euler);
}

void Spatial::set_rotation_degrees(const Vector3 euler_degrees) {
	___godot_icall_void_Vector3(___mb.mb_set_rotation_degrees, (const Object *) this, euler_degrees);
}

void Spatial::set_scale(const Vector3 scale) {
	___godot_icall_void_Vector3(___mb.mb_set_scale, (const Object *) this, scale);
}

void Spatial::set_transform(const Transform local) {
	___godot_icall_void_Transform(___mb.mb_set_transform, (const Object *) this, local);
}

void Spatial::set_translation(const Vector3 translation) {
	___godot_icall_void_Vector3(___mb.mb_set_translation, (const Object *) this, translation);
}

void Spatial::set_visible(const bool visible) {
	___godot_icall_void_bool(___mb.mb_set_visible, (const Object *) this, visible);
}

void Spatial::show() {
	___godot_icall_void(___mb.mb_show, (const Object *) this);
}

Vector3 Spatial::to_global(const Vector3 local_point) const {
	return ___godot_icall_Vector3_Vector3(___mb.mb_to_global, (const Object *) this, local_point);
}

Vector3 Spatial::to_local(const Vector3 global_point) const {
	return ___godot_icall_Vector3_Vector3(___mb.mb_to_local, (const Object *) this, global_point);
}

void Spatial::translate(const Vector3 offset) {
	___godot_icall_void_Vector3(___mb.mb_translate, (const Object *) this, offset);
}

void Spatial::translate_object_local(const Vector3 offset) {
	___godot_icall_void_Vector3(___mb.mb_translate_object_local, (const Object *) this, offset);
}

void Spatial::update_gizmo() {
	___godot_icall_void(___mb.mb_update_gizmo, (const Object *) this);
}

}