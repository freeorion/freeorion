#include "ParallaxBackground.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ParallaxBackground::___method_bindings ParallaxBackground::___mb = {};

void ParallaxBackground::___init_method_bindings() {
	___mb.mb__camera_moved = godot::api->godot_method_bind_get_method("ParallaxBackground", "_camera_moved");
	___mb.mb_get_limit_begin = godot::api->godot_method_bind_get_method("ParallaxBackground", "get_limit_begin");
	___mb.mb_get_limit_end = godot::api->godot_method_bind_get_method("ParallaxBackground", "get_limit_end");
	___mb.mb_get_scroll_base_offset = godot::api->godot_method_bind_get_method("ParallaxBackground", "get_scroll_base_offset");
	___mb.mb_get_scroll_base_scale = godot::api->godot_method_bind_get_method("ParallaxBackground", "get_scroll_base_scale");
	___mb.mb_get_scroll_offset = godot::api->godot_method_bind_get_method("ParallaxBackground", "get_scroll_offset");
	___mb.mb_is_ignore_camera_zoom = godot::api->godot_method_bind_get_method("ParallaxBackground", "is_ignore_camera_zoom");
	___mb.mb_set_ignore_camera_zoom = godot::api->godot_method_bind_get_method("ParallaxBackground", "set_ignore_camera_zoom");
	___mb.mb_set_limit_begin = godot::api->godot_method_bind_get_method("ParallaxBackground", "set_limit_begin");
	___mb.mb_set_limit_end = godot::api->godot_method_bind_get_method("ParallaxBackground", "set_limit_end");
	___mb.mb_set_scroll_base_offset = godot::api->godot_method_bind_get_method("ParallaxBackground", "set_scroll_base_offset");
	___mb.mb_set_scroll_base_scale = godot::api->godot_method_bind_get_method("ParallaxBackground", "set_scroll_base_scale");
	___mb.mb_set_scroll_offset = godot::api->godot_method_bind_get_method("ParallaxBackground", "set_scroll_offset");
}

ParallaxBackground *ParallaxBackground::_new()
{
	return (ParallaxBackground *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ParallaxBackground")());
}
void ParallaxBackground::_camera_moved(const Transform2D arg0, const Vector2 arg1) {
	___godot_icall_void_Transform2D_Vector2(___mb.mb__camera_moved, (const Object *) this, arg0, arg1);
}

Vector2 ParallaxBackground::get_limit_begin() const {
	return ___godot_icall_Vector2(___mb.mb_get_limit_begin, (const Object *) this);
}

Vector2 ParallaxBackground::get_limit_end() const {
	return ___godot_icall_Vector2(___mb.mb_get_limit_end, (const Object *) this);
}

Vector2 ParallaxBackground::get_scroll_base_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_scroll_base_offset, (const Object *) this);
}

Vector2 ParallaxBackground::get_scroll_base_scale() const {
	return ___godot_icall_Vector2(___mb.mb_get_scroll_base_scale, (const Object *) this);
}

Vector2 ParallaxBackground::get_scroll_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_scroll_offset, (const Object *) this);
}

bool ParallaxBackground::is_ignore_camera_zoom() {
	return ___godot_icall_bool(___mb.mb_is_ignore_camera_zoom, (const Object *) this);
}

void ParallaxBackground::set_ignore_camera_zoom(const bool ignore) {
	___godot_icall_void_bool(___mb.mb_set_ignore_camera_zoom, (const Object *) this, ignore);
}

void ParallaxBackground::set_limit_begin(const Vector2 ofs) {
	___godot_icall_void_Vector2(___mb.mb_set_limit_begin, (const Object *) this, ofs);
}

void ParallaxBackground::set_limit_end(const Vector2 ofs) {
	___godot_icall_void_Vector2(___mb.mb_set_limit_end, (const Object *) this, ofs);
}

void ParallaxBackground::set_scroll_base_offset(const Vector2 ofs) {
	___godot_icall_void_Vector2(___mb.mb_set_scroll_base_offset, (const Object *) this, ofs);
}

void ParallaxBackground::set_scroll_base_scale(const Vector2 scale) {
	___godot_icall_void_Vector2(___mb.mb_set_scroll_base_scale, (const Object *) this, scale);
}

void ParallaxBackground::set_scroll_offset(const Vector2 ofs) {
	___godot_icall_void_Vector2(___mb.mb_set_scroll_offset, (const Object *) this, ofs);
}

}