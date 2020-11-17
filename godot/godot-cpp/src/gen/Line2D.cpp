#include "Line2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Curve.hpp"
#include "Gradient.hpp"
#include "Texture.hpp"


namespace godot {


Line2D::___method_bindings Line2D::___mb = {};

void Line2D::___init_method_bindings() {
	___mb.mb__curve_changed = godot::api->godot_method_bind_get_method("Line2D", "_curve_changed");
	___mb.mb__gradient_changed = godot::api->godot_method_bind_get_method("Line2D", "_gradient_changed");
	___mb.mb_add_point = godot::api->godot_method_bind_get_method("Line2D", "add_point");
	___mb.mb_clear_points = godot::api->godot_method_bind_get_method("Line2D", "clear_points");
	___mb.mb_get_antialiased = godot::api->godot_method_bind_get_method("Line2D", "get_antialiased");
	___mb.mb_get_begin_cap_mode = godot::api->godot_method_bind_get_method("Line2D", "get_begin_cap_mode");
	___mb.mb_get_curve = godot::api->godot_method_bind_get_method("Line2D", "get_curve");
	___mb.mb_get_default_color = godot::api->godot_method_bind_get_method("Line2D", "get_default_color");
	___mb.mb_get_end_cap_mode = godot::api->godot_method_bind_get_method("Line2D", "get_end_cap_mode");
	___mb.mb_get_gradient = godot::api->godot_method_bind_get_method("Line2D", "get_gradient");
	___mb.mb_get_joint_mode = godot::api->godot_method_bind_get_method("Line2D", "get_joint_mode");
	___mb.mb_get_point_count = godot::api->godot_method_bind_get_method("Line2D", "get_point_count");
	___mb.mb_get_point_position = godot::api->godot_method_bind_get_method("Line2D", "get_point_position");
	___mb.mb_get_points = godot::api->godot_method_bind_get_method("Line2D", "get_points");
	___mb.mb_get_round_precision = godot::api->godot_method_bind_get_method("Line2D", "get_round_precision");
	___mb.mb_get_sharp_limit = godot::api->godot_method_bind_get_method("Line2D", "get_sharp_limit");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("Line2D", "get_texture");
	___mb.mb_get_texture_mode = godot::api->godot_method_bind_get_method("Line2D", "get_texture_mode");
	___mb.mb_get_width = godot::api->godot_method_bind_get_method("Line2D", "get_width");
	___mb.mb_remove_point = godot::api->godot_method_bind_get_method("Line2D", "remove_point");
	___mb.mb_set_antialiased = godot::api->godot_method_bind_get_method("Line2D", "set_antialiased");
	___mb.mb_set_begin_cap_mode = godot::api->godot_method_bind_get_method("Line2D", "set_begin_cap_mode");
	___mb.mb_set_curve = godot::api->godot_method_bind_get_method("Line2D", "set_curve");
	___mb.mb_set_default_color = godot::api->godot_method_bind_get_method("Line2D", "set_default_color");
	___mb.mb_set_end_cap_mode = godot::api->godot_method_bind_get_method("Line2D", "set_end_cap_mode");
	___mb.mb_set_gradient = godot::api->godot_method_bind_get_method("Line2D", "set_gradient");
	___mb.mb_set_joint_mode = godot::api->godot_method_bind_get_method("Line2D", "set_joint_mode");
	___mb.mb_set_point_position = godot::api->godot_method_bind_get_method("Line2D", "set_point_position");
	___mb.mb_set_points = godot::api->godot_method_bind_get_method("Line2D", "set_points");
	___mb.mb_set_round_precision = godot::api->godot_method_bind_get_method("Line2D", "set_round_precision");
	___mb.mb_set_sharp_limit = godot::api->godot_method_bind_get_method("Line2D", "set_sharp_limit");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("Line2D", "set_texture");
	___mb.mb_set_texture_mode = godot::api->godot_method_bind_get_method("Line2D", "set_texture_mode");
	___mb.mb_set_width = godot::api->godot_method_bind_get_method("Line2D", "set_width");
}

Line2D *Line2D::_new()
{
	return (Line2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Line2D")());
}
void Line2D::_curve_changed() {
	___godot_icall_void(___mb.mb__curve_changed, (const Object *) this);
}

void Line2D::_gradient_changed() {
	___godot_icall_void(___mb.mb__gradient_changed, (const Object *) this);
}

void Line2D::add_point(const Vector2 position, const int64_t at_position) {
	___godot_icall_void_Vector2_int(___mb.mb_add_point, (const Object *) this, position, at_position);
}

void Line2D::clear_points() {
	___godot_icall_void(___mb.mb_clear_points, (const Object *) this);
}

bool Line2D::get_antialiased() const {
	return ___godot_icall_bool(___mb.mb_get_antialiased, (const Object *) this);
}

Line2D::LineCapMode Line2D::get_begin_cap_mode() const {
	return (Line2D::LineCapMode) ___godot_icall_int(___mb.mb_get_begin_cap_mode, (const Object *) this);
}

Ref<Curve> Line2D::get_curve() const {
	return Ref<Curve>::__internal_constructor(___godot_icall_Object(___mb.mb_get_curve, (const Object *) this));
}

Color Line2D::get_default_color() const {
	return ___godot_icall_Color(___mb.mb_get_default_color, (const Object *) this);
}

Line2D::LineCapMode Line2D::get_end_cap_mode() const {
	return (Line2D::LineCapMode) ___godot_icall_int(___mb.mb_get_end_cap_mode, (const Object *) this);
}

Ref<Gradient> Line2D::get_gradient() const {
	return Ref<Gradient>::__internal_constructor(___godot_icall_Object(___mb.mb_get_gradient, (const Object *) this));
}

Line2D::LineJointMode Line2D::get_joint_mode() const {
	return (Line2D::LineJointMode) ___godot_icall_int(___mb.mb_get_joint_mode, (const Object *) this);
}

int64_t Line2D::get_point_count() const {
	return ___godot_icall_int(___mb.mb_get_point_count, (const Object *) this);
}

Vector2 Line2D::get_point_position(const int64_t i) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_point_position, (const Object *) this, i);
}

PoolVector2Array Line2D::get_points() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_points, (const Object *) this);
}

int64_t Line2D::get_round_precision() const {
	return ___godot_icall_int(___mb.mb_get_round_precision, (const Object *) this);
}

real_t Line2D::get_sharp_limit() const {
	return ___godot_icall_float(___mb.mb_get_sharp_limit, (const Object *) this);
}

Ref<Texture> Line2D::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

Line2D::LineTextureMode Line2D::get_texture_mode() const {
	return (Line2D::LineTextureMode) ___godot_icall_int(___mb.mb_get_texture_mode, (const Object *) this);
}

real_t Line2D::get_width() const {
	return ___godot_icall_float(___mb.mb_get_width, (const Object *) this);
}

void Line2D::remove_point(const int64_t i) {
	___godot_icall_void_int(___mb.mb_remove_point, (const Object *) this, i);
}

void Line2D::set_antialiased(const bool antialiased) {
	___godot_icall_void_bool(___mb.mb_set_antialiased, (const Object *) this, antialiased);
}

void Line2D::set_begin_cap_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_begin_cap_mode, (const Object *) this, mode);
}

void Line2D::set_curve(const Ref<Curve> curve) {
	___godot_icall_void_Object(___mb.mb_set_curve, (const Object *) this, curve.ptr());
}

void Line2D::set_default_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_default_color, (const Object *) this, color);
}

void Line2D::set_end_cap_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_end_cap_mode, (const Object *) this, mode);
}

void Line2D::set_gradient(const Ref<Gradient> color) {
	___godot_icall_void_Object(___mb.mb_set_gradient, (const Object *) this, color.ptr());
}

void Line2D::set_joint_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_joint_mode, (const Object *) this, mode);
}

void Line2D::set_point_position(const int64_t i, const Vector2 position) {
	___godot_icall_void_int_Vector2(___mb.mb_set_point_position, (const Object *) this, i, position);
}

void Line2D::set_points(const PoolVector2Array points) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_points, (const Object *) this, points);
}

void Line2D::set_round_precision(const int64_t precision) {
	___godot_icall_void_int(___mb.mb_set_round_precision, (const Object *) this, precision);
}

void Line2D::set_sharp_limit(const real_t limit) {
	___godot_icall_void_float(___mb.mb_set_sharp_limit, (const Object *) this, limit);
}

void Line2D::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void Line2D::set_texture_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_texture_mode, (const Object *) this, mode);
}

void Line2D::set_width(const real_t width) {
	___godot_icall_void_float(___mb.mb_set_width, (const Object *) this, width);
}

}