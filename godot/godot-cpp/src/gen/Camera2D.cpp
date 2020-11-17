#include "Camera2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "Node.hpp"


namespace godot {


Camera2D::___method_bindings Camera2D::___mb = {};

void Camera2D::___init_method_bindings() {
	___mb.mb__make_current = godot::api->godot_method_bind_get_method("Camera2D", "_make_current");
	___mb.mb__set_current = godot::api->godot_method_bind_get_method("Camera2D", "_set_current");
	___mb.mb__set_old_smoothing = godot::api->godot_method_bind_get_method("Camera2D", "_set_old_smoothing");
	___mb.mb__update_scroll = godot::api->godot_method_bind_get_method("Camera2D", "_update_scroll");
	___mb.mb_align = godot::api->godot_method_bind_get_method("Camera2D", "align");
	___mb.mb_clear_current = godot::api->godot_method_bind_get_method("Camera2D", "clear_current");
	___mb.mb_force_update_scroll = godot::api->godot_method_bind_get_method("Camera2D", "force_update_scroll");
	___mb.mb_get_anchor_mode = godot::api->godot_method_bind_get_method("Camera2D", "get_anchor_mode");
	___mb.mb_get_camera_position = godot::api->godot_method_bind_get_method("Camera2D", "get_camera_position");
	___mb.mb_get_camera_screen_center = godot::api->godot_method_bind_get_method("Camera2D", "get_camera_screen_center");
	___mb.mb_get_custom_viewport = godot::api->godot_method_bind_get_method("Camera2D", "get_custom_viewport");
	___mb.mb_get_drag_margin = godot::api->godot_method_bind_get_method("Camera2D", "get_drag_margin");
	___mb.mb_get_follow_smoothing = godot::api->godot_method_bind_get_method("Camera2D", "get_follow_smoothing");
	___mb.mb_get_h_offset = godot::api->godot_method_bind_get_method("Camera2D", "get_h_offset");
	___mb.mb_get_limit = godot::api->godot_method_bind_get_method("Camera2D", "get_limit");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("Camera2D", "get_offset");
	___mb.mb_get_process_mode = godot::api->godot_method_bind_get_method("Camera2D", "get_process_mode");
	___mb.mb_get_v_offset = godot::api->godot_method_bind_get_method("Camera2D", "get_v_offset");
	___mb.mb_get_zoom = godot::api->godot_method_bind_get_method("Camera2D", "get_zoom");
	___mb.mb_is_current = godot::api->godot_method_bind_get_method("Camera2D", "is_current");
	___mb.mb_is_follow_smoothing_enabled = godot::api->godot_method_bind_get_method("Camera2D", "is_follow_smoothing_enabled");
	___mb.mb_is_h_drag_enabled = godot::api->godot_method_bind_get_method("Camera2D", "is_h_drag_enabled");
	___mb.mb_is_limit_drawing_enabled = godot::api->godot_method_bind_get_method("Camera2D", "is_limit_drawing_enabled");
	___mb.mb_is_limit_smoothing_enabled = godot::api->godot_method_bind_get_method("Camera2D", "is_limit_smoothing_enabled");
	___mb.mb_is_margin_drawing_enabled = godot::api->godot_method_bind_get_method("Camera2D", "is_margin_drawing_enabled");
	___mb.mb_is_rotating = godot::api->godot_method_bind_get_method("Camera2D", "is_rotating");
	___mb.mb_is_screen_drawing_enabled = godot::api->godot_method_bind_get_method("Camera2D", "is_screen_drawing_enabled");
	___mb.mb_is_v_drag_enabled = godot::api->godot_method_bind_get_method("Camera2D", "is_v_drag_enabled");
	___mb.mb_make_current = godot::api->godot_method_bind_get_method("Camera2D", "make_current");
	___mb.mb_reset_smoothing = godot::api->godot_method_bind_get_method("Camera2D", "reset_smoothing");
	___mb.mb_set_anchor_mode = godot::api->godot_method_bind_get_method("Camera2D", "set_anchor_mode");
	___mb.mb_set_custom_viewport = godot::api->godot_method_bind_get_method("Camera2D", "set_custom_viewport");
	___mb.mb_set_drag_margin = godot::api->godot_method_bind_get_method("Camera2D", "set_drag_margin");
	___mb.mb_set_enable_follow_smoothing = godot::api->godot_method_bind_get_method("Camera2D", "set_enable_follow_smoothing");
	___mb.mb_set_follow_smoothing = godot::api->godot_method_bind_get_method("Camera2D", "set_follow_smoothing");
	___mb.mb_set_h_drag_enabled = godot::api->godot_method_bind_get_method("Camera2D", "set_h_drag_enabled");
	___mb.mb_set_h_offset = godot::api->godot_method_bind_get_method("Camera2D", "set_h_offset");
	___mb.mb_set_limit = godot::api->godot_method_bind_get_method("Camera2D", "set_limit");
	___mb.mb_set_limit_drawing_enabled = godot::api->godot_method_bind_get_method("Camera2D", "set_limit_drawing_enabled");
	___mb.mb_set_limit_smoothing_enabled = godot::api->godot_method_bind_get_method("Camera2D", "set_limit_smoothing_enabled");
	___mb.mb_set_margin_drawing_enabled = godot::api->godot_method_bind_get_method("Camera2D", "set_margin_drawing_enabled");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("Camera2D", "set_offset");
	___mb.mb_set_process_mode = godot::api->godot_method_bind_get_method("Camera2D", "set_process_mode");
	___mb.mb_set_rotating = godot::api->godot_method_bind_get_method("Camera2D", "set_rotating");
	___mb.mb_set_screen_drawing_enabled = godot::api->godot_method_bind_get_method("Camera2D", "set_screen_drawing_enabled");
	___mb.mb_set_v_drag_enabled = godot::api->godot_method_bind_get_method("Camera2D", "set_v_drag_enabled");
	___mb.mb_set_v_offset = godot::api->godot_method_bind_get_method("Camera2D", "set_v_offset");
	___mb.mb_set_zoom = godot::api->godot_method_bind_get_method("Camera2D", "set_zoom");
}

Camera2D *Camera2D::_new()
{
	return (Camera2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Camera2D")());
}
void Camera2D::_make_current(const Object *arg0) {
	___godot_icall_void_Object(___mb.mb__make_current, (const Object *) this, arg0);
}

void Camera2D::_set_current(const bool current) {
	___godot_icall_void_bool(___mb.mb__set_current, (const Object *) this, current);
}

void Camera2D::_set_old_smoothing(const real_t follow_smoothing) {
	___godot_icall_void_float(___mb.mb__set_old_smoothing, (const Object *) this, follow_smoothing);
}

void Camera2D::_update_scroll() {
	___godot_icall_void(___mb.mb__update_scroll, (const Object *) this);
}

void Camera2D::align() {
	___godot_icall_void(___mb.mb_align, (const Object *) this);
}

void Camera2D::clear_current() {
	___godot_icall_void(___mb.mb_clear_current, (const Object *) this);
}

void Camera2D::force_update_scroll() {
	___godot_icall_void(___mb.mb_force_update_scroll, (const Object *) this);
}

Camera2D::AnchorMode Camera2D::get_anchor_mode() const {
	return (Camera2D::AnchorMode) ___godot_icall_int(___mb.mb_get_anchor_mode, (const Object *) this);
}

Vector2 Camera2D::get_camera_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_camera_position, (const Object *) this);
}

Vector2 Camera2D::get_camera_screen_center() const {
	return ___godot_icall_Vector2(___mb.mb_get_camera_screen_center, (const Object *) this);
}

Node *Camera2D::get_custom_viewport() const {
	return (Node *) ___godot_icall_Object(___mb.mb_get_custom_viewport, (const Object *) this);
}

real_t Camera2D::get_drag_margin(const int64_t margin) const {
	return ___godot_icall_float_int(___mb.mb_get_drag_margin, (const Object *) this, margin);
}

real_t Camera2D::get_follow_smoothing() const {
	return ___godot_icall_float(___mb.mb_get_follow_smoothing, (const Object *) this);
}

real_t Camera2D::get_h_offset() const {
	return ___godot_icall_float(___mb.mb_get_h_offset, (const Object *) this);
}

int64_t Camera2D::get_limit(const int64_t margin) const {
	return ___godot_icall_int_int(___mb.mb_get_limit, (const Object *) this, margin);
}

Vector2 Camera2D::get_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_offset, (const Object *) this);
}

Camera2D::Camera2DProcessMode Camera2D::get_process_mode() const {
	return (Camera2D::Camera2DProcessMode) ___godot_icall_int(___mb.mb_get_process_mode, (const Object *) this);
}

real_t Camera2D::get_v_offset() const {
	return ___godot_icall_float(___mb.mb_get_v_offset, (const Object *) this);
}

Vector2 Camera2D::get_zoom() const {
	return ___godot_icall_Vector2(___mb.mb_get_zoom, (const Object *) this);
}

bool Camera2D::is_current() const {
	return ___godot_icall_bool(___mb.mb_is_current, (const Object *) this);
}

bool Camera2D::is_follow_smoothing_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_follow_smoothing_enabled, (const Object *) this);
}

bool Camera2D::is_h_drag_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_h_drag_enabled, (const Object *) this);
}

bool Camera2D::is_limit_drawing_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_limit_drawing_enabled, (const Object *) this);
}

bool Camera2D::is_limit_smoothing_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_limit_smoothing_enabled, (const Object *) this);
}

bool Camera2D::is_margin_drawing_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_margin_drawing_enabled, (const Object *) this);
}

bool Camera2D::is_rotating() const {
	return ___godot_icall_bool(___mb.mb_is_rotating, (const Object *) this);
}

bool Camera2D::is_screen_drawing_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_screen_drawing_enabled, (const Object *) this);
}

bool Camera2D::is_v_drag_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_v_drag_enabled, (const Object *) this);
}

void Camera2D::make_current() {
	___godot_icall_void(___mb.mb_make_current, (const Object *) this);
}

void Camera2D::reset_smoothing() {
	___godot_icall_void(___mb.mb_reset_smoothing, (const Object *) this);
}

void Camera2D::set_anchor_mode(const int64_t anchor_mode) {
	___godot_icall_void_int(___mb.mb_set_anchor_mode, (const Object *) this, anchor_mode);
}

void Camera2D::set_custom_viewport(const Node *viewport) {
	___godot_icall_void_Object(___mb.mb_set_custom_viewport, (const Object *) this, viewport);
}

void Camera2D::set_drag_margin(const int64_t margin, const real_t drag_margin) {
	___godot_icall_void_int_float(___mb.mb_set_drag_margin, (const Object *) this, margin, drag_margin);
}

void Camera2D::set_enable_follow_smoothing(const bool follow_smoothing) {
	___godot_icall_void_bool(___mb.mb_set_enable_follow_smoothing, (const Object *) this, follow_smoothing);
}

void Camera2D::set_follow_smoothing(const real_t follow_smoothing) {
	___godot_icall_void_float(___mb.mb_set_follow_smoothing, (const Object *) this, follow_smoothing);
}

void Camera2D::set_h_drag_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_h_drag_enabled, (const Object *) this, enabled);
}

void Camera2D::set_h_offset(const real_t ofs) {
	___godot_icall_void_float(___mb.mb_set_h_offset, (const Object *) this, ofs);
}

void Camera2D::set_limit(const int64_t margin, const int64_t limit) {
	___godot_icall_void_int_int(___mb.mb_set_limit, (const Object *) this, margin, limit);
}

void Camera2D::set_limit_drawing_enabled(const bool limit_drawing_enabled) {
	___godot_icall_void_bool(___mb.mb_set_limit_drawing_enabled, (const Object *) this, limit_drawing_enabled);
}

void Camera2D::set_limit_smoothing_enabled(const bool limit_smoothing_enabled) {
	___godot_icall_void_bool(___mb.mb_set_limit_smoothing_enabled, (const Object *) this, limit_smoothing_enabled);
}

void Camera2D::set_margin_drawing_enabled(const bool margin_drawing_enabled) {
	___godot_icall_void_bool(___mb.mb_set_margin_drawing_enabled, (const Object *) this, margin_drawing_enabled);
}

void Camera2D::set_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_offset, (const Object *) this, offset);
}

void Camera2D::set_process_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_process_mode, (const Object *) this, mode);
}

void Camera2D::set_rotating(const bool rotating) {
	___godot_icall_void_bool(___mb.mb_set_rotating, (const Object *) this, rotating);
}

void Camera2D::set_screen_drawing_enabled(const bool screen_drawing_enabled) {
	___godot_icall_void_bool(___mb.mb_set_screen_drawing_enabled, (const Object *) this, screen_drawing_enabled);
}

void Camera2D::set_v_drag_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_v_drag_enabled, (const Object *) this, enabled);
}

void Camera2D::set_v_offset(const real_t ofs) {
	___godot_icall_void_float(___mb.mb_set_v_offset, (const Object *) this, ofs);
}

void Camera2D::set_zoom(const Vector2 zoom) {
	___godot_icall_void_Vector2(___mb.mb_set_zoom, (const Object *) this, zoom);
}

}