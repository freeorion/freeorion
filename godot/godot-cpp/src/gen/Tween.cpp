#include "Tween.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


Tween::___method_bindings Tween::___mb = {};

void Tween::___init_method_bindings() {
	___mb.mb__remove_by_uid = godot::api->godot_method_bind_get_method("Tween", "_remove_by_uid");
	___mb.mb_follow_method = godot::api->godot_method_bind_get_method("Tween", "follow_method");
	___mb.mb_follow_property = godot::api->godot_method_bind_get_method("Tween", "follow_property");
	___mb.mb_get_runtime = godot::api->godot_method_bind_get_method("Tween", "get_runtime");
	___mb.mb_get_speed_scale = godot::api->godot_method_bind_get_method("Tween", "get_speed_scale");
	___mb.mb_get_tween_process_mode = godot::api->godot_method_bind_get_method("Tween", "get_tween_process_mode");
	___mb.mb_interpolate_callback = godot::api->godot_method_bind_get_method("Tween", "interpolate_callback");
	___mb.mb_interpolate_deferred_callback = godot::api->godot_method_bind_get_method("Tween", "interpolate_deferred_callback");
	___mb.mb_interpolate_method = godot::api->godot_method_bind_get_method("Tween", "interpolate_method");
	___mb.mb_interpolate_property = godot::api->godot_method_bind_get_method("Tween", "interpolate_property");
	___mb.mb_is_active = godot::api->godot_method_bind_get_method("Tween", "is_active");
	___mb.mb_is_repeat = godot::api->godot_method_bind_get_method("Tween", "is_repeat");
	___mb.mb_remove = godot::api->godot_method_bind_get_method("Tween", "remove");
	___mb.mb_remove_all = godot::api->godot_method_bind_get_method("Tween", "remove_all");
	___mb.mb_reset = godot::api->godot_method_bind_get_method("Tween", "reset");
	___mb.mb_reset_all = godot::api->godot_method_bind_get_method("Tween", "reset_all");
	___mb.mb_resume = godot::api->godot_method_bind_get_method("Tween", "resume");
	___mb.mb_resume_all = godot::api->godot_method_bind_get_method("Tween", "resume_all");
	___mb.mb_seek = godot::api->godot_method_bind_get_method("Tween", "seek");
	___mb.mb_set_active = godot::api->godot_method_bind_get_method("Tween", "set_active");
	___mb.mb_set_repeat = godot::api->godot_method_bind_get_method("Tween", "set_repeat");
	___mb.mb_set_speed_scale = godot::api->godot_method_bind_get_method("Tween", "set_speed_scale");
	___mb.mb_set_tween_process_mode = godot::api->godot_method_bind_get_method("Tween", "set_tween_process_mode");
	___mb.mb_start = godot::api->godot_method_bind_get_method("Tween", "start");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("Tween", "stop");
	___mb.mb_stop_all = godot::api->godot_method_bind_get_method("Tween", "stop_all");
	___mb.mb_targeting_method = godot::api->godot_method_bind_get_method("Tween", "targeting_method");
	___mb.mb_targeting_property = godot::api->godot_method_bind_get_method("Tween", "targeting_property");
	___mb.mb_tell = godot::api->godot_method_bind_get_method("Tween", "tell");
}

Tween *Tween::_new()
{
	return (Tween *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Tween")());
}
void Tween::_remove_by_uid(const int64_t uid) {
	___godot_icall_void_int(___mb.mb__remove_by_uid, (const Object *) this, uid);
}

bool Tween::follow_method(const Object *object, const String method, const Variant initial_val, const Object *target, const String target_method, const real_t duration, const int64_t trans_type, const int64_t ease_type, const real_t delay) {
	return ___godot_icall_bool_Object_String_Variant_Object_String_float_int_int_float(___mb.mb_follow_method, (const Object *) this, object, method, initial_val, target, target_method, duration, trans_type, ease_type, delay);
}

bool Tween::follow_property(const Object *object, const NodePath property, const Variant initial_val, const Object *target, const NodePath target_property, const real_t duration, const int64_t trans_type, const int64_t ease_type, const real_t delay) {
	return ___godot_icall_bool_Object_NodePath_Variant_Object_NodePath_float_int_int_float(___mb.mb_follow_property, (const Object *) this, object, property, initial_val, target, target_property, duration, trans_type, ease_type, delay);
}

real_t Tween::get_runtime() const {
	return ___godot_icall_float(___mb.mb_get_runtime, (const Object *) this);
}

real_t Tween::get_speed_scale() const {
	return ___godot_icall_float(___mb.mb_get_speed_scale, (const Object *) this);
}

Tween::TweenProcessMode Tween::get_tween_process_mode() const {
	return (Tween::TweenProcessMode) ___godot_icall_int(___mb.mb_get_tween_process_mode, (const Object *) this);
}

bool Tween::interpolate_callback(const Object *object, const real_t duration, const String callback, const Variant arg1, const Variant arg2, const Variant arg3, const Variant arg4, const Variant arg5) {
	return ___godot_icall_bool_Object_float_String_Variant_Variant_Variant_Variant_Variant(___mb.mb_interpolate_callback, (const Object *) this, object, duration, callback, arg1, arg2, arg3, arg4, arg5);
}

bool Tween::interpolate_deferred_callback(const Object *object, const real_t duration, const String callback, const Variant arg1, const Variant arg2, const Variant arg3, const Variant arg4, const Variant arg5) {
	return ___godot_icall_bool_Object_float_String_Variant_Variant_Variant_Variant_Variant(___mb.mb_interpolate_deferred_callback, (const Object *) this, object, duration, callback, arg1, arg2, arg3, arg4, arg5);
}

bool Tween::interpolate_method(const Object *object, const String method, const Variant initial_val, const Variant final_val, const real_t duration, const int64_t trans_type, const int64_t ease_type, const real_t delay) {
	return ___godot_icall_bool_Object_String_Variant_Variant_float_int_int_float(___mb.mb_interpolate_method, (const Object *) this, object, method, initial_val, final_val, duration, trans_type, ease_type, delay);
}

bool Tween::interpolate_property(const Object *object, const NodePath property, const Variant initial_val, const Variant final_val, const real_t duration, const int64_t trans_type, const int64_t ease_type, const real_t delay) {
	return ___godot_icall_bool_Object_NodePath_Variant_Variant_float_int_int_float(___mb.mb_interpolate_property, (const Object *) this, object, property, initial_val, final_val, duration, trans_type, ease_type, delay);
}

bool Tween::is_active() const {
	return ___godot_icall_bool(___mb.mb_is_active, (const Object *) this);
}

bool Tween::is_repeat() const {
	return ___godot_icall_bool(___mb.mb_is_repeat, (const Object *) this);
}

bool Tween::remove(const Object *object, const String key) {
	return ___godot_icall_bool_Object_String(___mb.mb_remove, (const Object *) this, object, key);
}

bool Tween::remove_all() {
	return ___godot_icall_bool(___mb.mb_remove_all, (const Object *) this);
}

bool Tween::reset(const Object *object, const String key) {
	return ___godot_icall_bool_Object_String(___mb.mb_reset, (const Object *) this, object, key);
}

bool Tween::reset_all() {
	return ___godot_icall_bool(___mb.mb_reset_all, (const Object *) this);
}

bool Tween::resume(const Object *object, const String key) {
	return ___godot_icall_bool_Object_String(___mb.mb_resume, (const Object *) this, object, key);
}

bool Tween::resume_all() {
	return ___godot_icall_bool(___mb.mb_resume_all, (const Object *) this);
}

bool Tween::seek(const real_t time) {
	return ___godot_icall_bool_float(___mb.mb_seek, (const Object *) this, time);
}

void Tween::set_active(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_active, (const Object *) this, active);
}

void Tween::set_repeat(const bool repeat) {
	___godot_icall_void_bool(___mb.mb_set_repeat, (const Object *) this, repeat);
}

void Tween::set_speed_scale(const real_t speed) {
	___godot_icall_void_float(___mb.mb_set_speed_scale, (const Object *) this, speed);
}

void Tween::set_tween_process_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_tween_process_mode, (const Object *) this, mode);
}

bool Tween::start() {
	return ___godot_icall_bool(___mb.mb_start, (const Object *) this);
}

bool Tween::stop(const Object *object, const String key) {
	return ___godot_icall_bool_Object_String(___mb.mb_stop, (const Object *) this, object, key);
}

bool Tween::stop_all() {
	return ___godot_icall_bool(___mb.mb_stop_all, (const Object *) this);
}

bool Tween::targeting_method(const Object *object, const String method, const Object *initial, const String initial_method, const Variant final_val, const real_t duration, const int64_t trans_type, const int64_t ease_type, const real_t delay) {
	return ___godot_icall_bool_Object_String_Object_String_Variant_float_int_int_float(___mb.mb_targeting_method, (const Object *) this, object, method, initial, initial_method, final_val, duration, trans_type, ease_type, delay);
}

bool Tween::targeting_property(const Object *object, const NodePath property, const Object *initial, const NodePath initial_val, const Variant final_val, const real_t duration, const int64_t trans_type, const int64_t ease_type, const real_t delay) {
	return ___godot_icall_bool_Object_NodePath_Object_NodePath_Variant_float_int_int_float(___mb.mb_targeting_property, (const Object *) this, object, property, initial, initial_val, final_val, duration, trans_type, ease_type, delay);
}

real_t Tween::tell() const {
	return ___godot_icall_float(___mb.mb_tell, (const Object *) this);
}

}