#include "ClippedCamera.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


ClippedCamera::___method_bindings ClippedCamera::___mb = {};

void ClippedCamera::___init_method_bindings() {
	___mb.mb_add_exception = godot::api->godot_method_bind_get_method("ClippedCamera", "add_exception");
	___mb.mb_add_exception_rid = godot::api->godot_method_bind_get_method("ClippedCamera", "add_exception_rid");
	___mb.mb_clear_exceptions = godot::api->godot_method_bind_get_method("ClippedCamera", "clear_exceptions");
	___mb.mb_get_clip_offset = godot::api->godot_method_bind_get_method("ClippedCamera", "get_clip_offset");
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("ClippedCamera", "get_collision_mask");
	___mb.mb_get_collision_mask_bit = godot::api->godot_method_bind_get_method("ClippedCamera", "get_collision_mask_bit");
	___mb.mb_get_margin = godot::api->godot_method_bind_get_method("ClippedCamera", "get_margin");
	___mb.mb_get_process_mode = godot::api->godot_method_bind_get_method("ClippedCamera", "get_process_mode");
	___mb.mb_is_clip_to_areas_enabled = godot::api->godot_method_bind_get_method("ClippedCamera", "is_clip_to_areas_enabled");
	___mb.mb_is_clip_to_bodies_enabled = godot::api->godot_method_bind_get_method("ClippedCamera", "is_clip_to_bodies_enabled");
	___mb.mb_remove_exception = godot::api->godot_method_bind_get_method("ClippedCamera", "remove_exception");
	___mb.mb_remove_exception_rid = godot::api->godot_method_bind_get_method("ClippedCamera", "remove_exception_rid");
	___mb.mb_set_clip_to_areas = godot::api->godot_method_bind_get_method("ClippedCamera", "set_clip_to_areas");
	___mb.mb_set_clip_to_bodies = godot::api->godot_method_bind_get_method("ClippedCamera", "set_clip_to_bodies");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("ClippedCamera", "set_collision_mask");
	___mb.mb_set_collision_mask_bit = godot::api->godot_method_bind_get_method("ClippedCamera", "set_collision_mask_bit");
	___mb.mb_set_margin = godot::api->godot_method_bind_get_method("ClippedCamera", "set_margin");
	___mb.mb_set_process_mode = godot::api->godot_method_bind_get_method("ClippedCamera", "set_process_mode");
}

ClippedCamera *ClippedCamera::_new()
{
	return (ClippedCamera *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ClippedCamera")());
}
void ClippedCamera::add_exception(const Object *node) {
	___godot_icall_void_Object(___mb.mb_add_exception, (const Object *) this, node);
}

void ClippedCamera::add_exception_rid(const RID rid) {
	___godot_icall_void_RID(___mb.mb_add_exception_rid, (const Object *) this, rid);
}

void ClippedCamera::clear_exceptions() {
	___godot_icall_void(___mb.mb_clear_exceptions, (const Object *) this);
}

real_t ClippedCamera::get_clip_offset() const {
	return ___godot_icall_float(___mb.mb_get_clip_offset, (const Object *) this);
}

int64_t ClippedCamera::get_collision_mask() const {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

bool ClippedCamera::get_collision_mask_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_mask_bit, (const Object *) this, bit);
}

real_t ClippedCamera::get_margin() const {
	return ___godot_icall_float(___mb.mb_get_margin, (const Object *) this);
}

ClippedCamera::ProcessMode ClippedCamera::get_process_mode() const {
	return (ClippedCamera::ProcessMode) ___godot_icall_int(___mb.mb_get_process_mode, (const Object *) this);
}

bool ClippedCamera::is_clip_to_areas_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_clip_to_areas_enabled, (const Object *) this);
}

bool ClippedCamera::is_clip_to_bodies_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_clip_to_bodies_enabled, (const Object *) this);
}

void ClippedCamera::remove_exception(const Object *node) {
	___godot_icall_void_Object(___mb.mb_remove_exception, (const Object *) this, node);
}

void ClippedCamera::remove_exception_rid(const RID rid) {
	___godot_icall_void_RID(___mb.mb_remove_exception_rid, (const Object *) this, rid);
}

void ClippedCamera::set_clip_to_areas(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_clip_to_areas, (const Object *) this, enable);
}

void ClippedCamera::set_clip_to_bodies(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_clip_to_bodies, (const Object *) this, enable);
}

void ClippedCamera::set_collision_mask(const int64_t mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, mask);
}

void ClippedCamera::set_collision_mask_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_mask_bit, (const Object *) this, bit, value);
}

void ClippedCamera::set_margin(const real_t margin) {
	___godot_icall_void_float(___mb.mb_set_margin, (const Object *) this, margin);
}

void ClippedCamera::set_process_mode(const int64_t process_mode) {
	___godot_icall_void_int(___mb.mb_set_process_mode, (const Object *) this, process_mode);
}

}