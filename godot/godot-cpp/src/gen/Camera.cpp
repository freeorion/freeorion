#include "Camera.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Environment.hpp"


namespace godot {


Camera::___method_bindings Camera::___mb = {};

void Camera::___init_method_bindings() {
	___mb.mb_clear_current = godot::api->godot_method_bind_get_method("Camera", "clear_current");
	___mb.mb_get_camera_rid = godot::api->godot_method_bind_get_method("Camera", "get_camera_rid");
	___mb.mb_get_camera_transform = godot::api->godot_method_bind_get_method("Camera", "get_camera_transform");
	___mb.mb_get_cull_mask = godot::api->godot_method_bind_get_method("Camera", "get_cull_mask");
	___mb.mb_get_cull_mask_bit = godot::api->godot_method_bind_get_method("Camera", "get_cull_mask_bit");
	___mb.mb_get_doppler_tracking = godot::api->godot_method_bind_get_method("Camera", "get_doppler_tracking");
	___mb.mb_get_environment = godot::api->godot_method_bind_get_method("Camera", "get_environment");
	___mb.mb_get_fov = godot::api->godot_method_bind_get_method("Camera", "get_fov");
	___mb.mb_get_frustum = godot::api->godot_method_bind_get_method("Camera", "get_frustum");
	___mb.mb_get_frustum_offset = godot::api->godot_method_bind_get_method("Camera", "get_frustum_offset");
	___mb.mb_get_h_offset = godot::api->godot_method_bind_get_method("Camera", "get_h_offset");
	___mb.mb_get_keep_aspect_mode = godot::api->godot_method_bind_get_method("Camera", "get_keep_aspect_mode");
	___mb.mb_get_projection = godot::api->godot_method_bind_get_method("Camera", "get_projection");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("Camera", "get_size");
	___mb.mb_get_v_offset = godot::api->godot_method_bind_get_method("Camera", "get_v_offset");
	___mb.mb_get_zfar = godot::api->godot_method_bind_get_method("Camera", "get_zfar");
	___mb.mb_get_znear = godot::api->godot_method_bind_get_method("Camera", "get_znear");
	___mb.mb_is_current = godot::api->godot_method_bind_get_method("Camera", "is_current");
	___mb.mb_is_position_behind = godot::api->godot_method_bind_get_method("Camera", "is_position_behind");
	___mb.mb_make_current = godot::api->godot_method_bind_get_method("Camera", "make_current");
	___mb.mb_project_local_ray_normal = godot::api->godot_method_bind_get_method("Camera", "project_local_ray_normal");
	___mb.mb_project_position = godot::api->godot_method_bind_get_method("Camera", "project_position");
	___mb.mb_project_ray_normal = godot::api->godot_method_bind_get_method("Camera", "project_ray_normal");
	___mb.mb_project_ray_origin = godot::api->godot_method_bind_get_method("Camera", "project_ray_origin");
	___mb.mb_set_cull_mask = godot::api->godot_method_bind_get_method("Camera", "set_cull_mask");
	___mb.mb_set_cull_mask_bit = godot::api->godot_method_bind_get_method("Camera", "set_cull_mask_bit");
	___mb.mb_set_current = godot::api->godot_method_bind_get_method("Camera", "set_current");
	___mb.mb_set_doppler_tracking = godot::api->godot_method_bind_get_method("Camera", "set_doppler_tracking");
	___mb.mb_set_environment = godot::api->godot_method_bind_get_method("Camera", "set_environment");
	___mb.mb_set_fov = godot::api->godot_method_bind_get_method("Camera", "set_fov");
	___mb.mb_set_frustum = godot::api->godot_method_bind_get_method("Camera", "set_frustum");
	___mb.mb_set_frustum_offset = godot::api->godot_method_bind_get_method("Camera", "set_frustum_offset");
	___mb.mb_set_h_offset = godot::api->godot_method_bind_get_method("Camera", "set_h_offset");
	___mb.mb_set_keep_aspect_mode = godot::api->godot_method_bind_get_method("Camera", "set_keep_aspect_mode");
	___mb.mb_set_orthogonal = godot::api->godot_method_bind_get_method("Camera", "set_orthogonal");
	___mb.mb_set_perspective = godot::api->godot_method_bind_get_method("Camera", "set_perspective");
	___mb.mb_set_projection = godot::api->godot_method_bind_get_method("Camera", "set_projection");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("Camera", "set_size");
	___mb.mb_set_v_offset = godot::api->godot_method_bind_get_method("Camera", "set_v_offset");
	___mb.mb_set_zfar = godot::api->godot_method_bind_get_method("Camera", "set_zfar");
	___mb.mb_set_znear = godot::api->godot_method_bind_get_method("Camera", "set_znear");
	___mb.mb_unproject_position = godot::api->godot_method_bind_get_method("Camera", "unproject_position");
}

Camera *Camera::_new()
{
	return (Camera *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Camera")());
}
void Camera::clear_current(const bool enable_next) {
	___godot_icall_void_bool(___mb.mb_clear_current, (const Object *) this, enable_next);
}

RID Camera::get_camera_rid() const {
	return ___godot_icall_RID(___mb.mb_get_camera_rid, (const Object *) this);
}

Transform Camera::get_camera_transform() const {
	return ___godot_icall_Transform(___mb.mb_get_camera_transform, (const Object *) this);
}

int64_t Camera::get_cull_mask() const {
	return ___godot_icall_int(___mb.mb_get_cull_mask, (const Object *) this);
}

bool Camera::get_cull_mask_bit(const int64_t layer) const {
	return ___godot_icall_bool_int(___mb.mb_get_cull_mask_bit, (const Object *) this, layer);
}

Camera::DopplerTracking Camera::get_doppler_tracking() const {
	return (Camera::DopplerTracking) ___godot_icall_int(___mb.mb_get_doppler_tracking, (const Object *) this);
}

Ref<Environment> Camera::get_environment() const {
	return Ref<Environment>::__internal_constructor(___godot_icall_Object(___mb.mb_get_environment, (const Object *) this));
}

real_t Camera::get_fov() const {
	return ___godot_icall_float(___mb.mb_get_fov, (const Object *) this);
}

Array Camera::get_frustum() const {
	return ___godot_icall_Array(___mb.mb_get_frustum, (const Object *) this);
}

Vector2 Camera::get_frustum_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_frustum_offset, (const Object *) this);
}

real_t Camera::get_h_offset() const {
	return ___godot_icall_float(___mb.mb_get_h_offset, (const Object *) this);
}

Camera::KeepAspect Camera::get_keep_aspect_mode() const {
	return (Camera::KeepAspect) ___godot_icall_int(___mb.mb_get_keep_aspect_mode, (const Object *) this);
}

Camera::Projection Camera::get_projection() const {
	return (Camera::Projection) ___godot_icall_int(___mb.mb_get_projection, (const Object *) this);
}

real_t Camera::get_size() const {
	return ___godot_icall_float(___mb.mb_get_size, (const Object *) this);
}

real_t Camera::get_v_offset() const {
	return ___godot_icall_float(___mb.mb_get_v_offset, (const Object *) this);
}

real_t Camera::get_zfar() const {
	return ___godot_icall_float(___mb.mb_get_zfar, (const Object *) this);
}

real_t Camera::get_znear() const {
	return ___godot_icall_float(___mb.mb_get_znear, (const Object *) this);
}

bool Camera::is_current() const {
	return ___godot_icall_bool(___mb.mb_is_current, (const Object *) this);
}

bool Camera::is_position_behind(const Vector3 world_point) const {
	return ___godot_icall_bool_Vector3(___mb.mb_is_position_behind, (const Object *) this, world_point);
}

void Camera::make_current() {
	___godot_icall_void(___mb.mb_make_current, (const Object *) this);
}

Vector3 Camera::project_local_ray_normal(const Vector2 screen_point) const {
	return ___godot_icall_Vector3_Vector2(___mb.mb_project_local_ray_normal, (const Object *) this, screen_point);
}

Vector3 Camera::project_position(const Vector2 screen_point, const real_t z_depth) const {
	return ___godot_icall_Vector3_Vector2_float(___mb.mb_project_position, (const Object *) this, screen_point, z_depth);
}

Vector3 Camera::project_ray_normal(const Vector2 screen_point) const {
	return ___godot_icall_Vector3_Vector2(___mb.mb_project_ray_normal, (const Object *) this, screen_point);
}

Vector3 Camera::project_ray_origin(const Vector2 screen_point) const {
	return ___godot_icall_Vector3_Vector2(___mb.mb_project_ray_origin, (const Object *) this, screen_point);
}

void Camera::set_cull_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_cull_mask, (const Object *) this, mask);
}

void Camera::set_cull_mask_bit(const int64_t layer, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_cull_mask_bit, (const Object *) this, layer, enable);
}

void Camera::set_current(const bool arg0) {
	___godot_icall_void_bool(___mb.mb_set_current, (const Object *) this, arg0);
}

void Camera::set_doppler_tracking(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_doppler_tracking, (const Object *) this, mode);
}

void Camera::set_environment(const Ref<Environment> env) {
	___godot_icall_void_Object(___mb.mb_set_environment, (const Object *) this, env.ptr());
}

void Camera::set_fov(const real_t arg0) {
	___godot_icall_void_float(___mb.mb_set_fov, (const Object *) this, arg0);
}

void Camera::set_frustum(const real_t size, const Vector2 offset, const real_t z_near, const real_t z_far) {
	___godot_icall_void_float_Vector2_float_float(___mb.mb_set_frustum, (const Object *) this, size, offset, z_near, z_far);
}

void Camera::set_frustum_offset(const Vector2 arg0) {
	___godot_icall_void_Vector2(___mb.mb_set_frustum_offset, (const Object *) this, arg0);
}

void Camera::set_h_offset(const real_t ofs) {
	___godot_icall_void_float(___mb.mb_set_h_offset, (const Object *) this, ofs);
}

void Camera::set_keep_aspect_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_keep_aspect_mode, (const Object *) this, mode);
}

void Camera::set_orthogonal(const real_t size, const real_t z_near, const real_t z_far) {
	___godot_icall_void_float_float_float(___mb.mb_set_orthogonal, (const Object *) this, size, z_near, z_far);
}

void Camera::set_perspective(const real_t fov, const real_t z_near, const real_t z_far) {
	___godot_icall_void_float_float_float(___mb.mb_set_perspective, (const Object *) this, fov, z_near, z_far);
}

void Camera::set_projection(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb_set_projection, (const Object *) this, arg0);
}

void Camera::set_size(const real_t arg0) {
	___godot_icall_void_float(___mb.mb_set_size, (const Object *) this, arg0);
}

void Camera::set_v_offset(const real_t ofs) {
	___godot_icall_void_float(___mb.mb_set_v_offset, (const Object *) this, ofs);
}

void Camera::set_zfar(const real_t arg0) {
	___godot_icall_void_float(___mb.mb_set_zfar, (const Object *) this, arg0);
}

void Camera::set_znear(const real_t arg0) {
	___godot_icall_void_float(___mb.mb_set_znear, (const Object *) this, arg0);
}

Vector2 Camera::unproject_position(const Vector3 world_point) const {
	return ___godot_icall_Vector2_Vector3(___mb.mb_unproject_position, (const Object *) this, world_point);
}

}