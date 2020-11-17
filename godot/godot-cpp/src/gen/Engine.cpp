#include "Engine.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "MainLoop.hpp"
#include "Object.hpp"


namespace godot {


Engine *Engine::_singleton = NULL;


Engine::Engine() {
	_owner = godot::api->godot_global_get_singleton((char *) "Engine");
}


Engine::___method_bindings Engine::___mb = {};

void Engine::___init_method_bindings() {
	___mb.mb_get_author_info = godot::api->godot_method_bind_get_method("_Engine", "get_author_info");
	___mb.mb_get_copyright_info = godot::api->godot_method_bind_get_method("_Engine", "get_copyright_info");
	___mb.mb_get_donor_info = godot::api->godot_method_bind_get_method("_Engine", "get_donor_info");
	___mb.mb_get_frames_drawn = godot::api->godot_method_bind_get_method("_Engine", "get_frames_drawn");
	___mb.mb_get_frames_per_second = godot::api->godot_method_bind_get_method("_Engine", "get_frames_per_second");
	___mb.mb_get_idle_frames = godot::api->godot_method_bind_get_method("_Engine", "get_idle_frames");
	___mb.mb_get_iterations_per_second = godot::api->godot_method_bind_get_method("_Engine", "get_iterations_per_second");
	___mb.mb_get_license_info = godot::api->godot_method_bind_get_method("_Engine", "get_license_info");
	___mb.mb_get_license_text = godot::api->godot_method_bind_get_method("_Engine", "get_license_text");
	___mb.mb_get_main_loop = godot::api->godot_method_bind_get_method("_Engine", "get_main_loop");
	___mb.mb_get_physics_frames = godot::api->godot_method_bind_get_method("_Engine", "get_physics_frames");
	___mb.mb_get_physics_interpolation_fraction = godot::api->godot_method_bind_get_method("_Engine", "get_physics_interpolation_fraction");
	___mb.mb_get_physics_jitter_fix = godot::api->godot_method_bind_get_method("_Engine", "get_physics_jitter_fix");
	___mb.mb_get_singleton = godot::api->godot_method_bind_get_method("_Engine", "get_singleton");
	___mb.mb_get_target_fps = godot::api->godot_method_bind_get_method("_Engine", "get_target_fps");
	___mb.mb_get_time_scale = godot::api->godot_method_bind_get_method("_Engine", "get_time_scale");
	___mb.mb_get_version_info = godot::api->godot_method_bind_get_method("_Engine", "get_version_info");
	___mb.mb_has_singleton = godot::api->godot_method_bind_get_method("_Engine", "has_singleton");
	___mb.mb_is_editor_hint = godot::api->godot_method_bind_get_method("_Engine", "is_editor_hint");
	___mb.mb_is_in_physics_frame = godot::api->godot_method_bind_get_method("_Engine", "is_in_physics_frame");
	___mb.mb_set_editor_hint = godot::api->godot_method_bind_get_method("_Engine", "set_editor_hint");
	___mb.mb_set_iterations_per_second = godot::api->godot_method_bind_get_method("_Engine", "set_iterations_per_second");
	___mb.mb_set_physics_jitter_fix = godot::api->godot_method_bind_get_method("_Engine", "set_physics_jitter_fix");
	___mb.mb_set_target_fps = godot::api->godot_method_bind_get_method("_Engine", "set_target_fps");
	___mb.mb_set_time_scale = godot::api->godot_method_bind_get_method("_Engine", "set_time_scale");
}

Dictionary Engine::get_author_info() const {
	return ___godot_icall_Dictionary(___mb.mb_get_author_info, (const Object *) this);
}

Array Engine::get_copyright_info() const {
	return ___godot_icall_Array(___mb.mb_get_copyright_info, (const Object *) this);
}

Dictionary Engine::get_donor_info() const {
	return ___godot_icall_Dictionary(___mb.mb_get_donor_info, (const Object *) this);
}

int64_t Engine::get_frames_drawn() {
	return ___godot_icall_int(___mb.mb_get_frames_drawn, (const Object *) this);
}

real_t Engine::get_frames_per_second() const {
	return ___godot_icall_float(___mb.mb_get_frames_per_second, (const Object *) this);
}

int64_t Engine::get_idle_frames() const {
	return ___godot_icall_int(___mb.mb_get_idle_frames, (const Object *) this);
}

int64_t Engine::get_iterations_per_second() const {
	return ___godot_icall_int(___mb.mb_get_iterations_per_second, (const Object *) this);
}

Dictionary Engine::get_license_info() const {
	return ___godot_icall_Dictionary(___mb.mb_get_license_info, (const Object *) this);
}

String Engine::get_license_text() const {
	return ___godot_icall_String(___mb.mb_get_license_text, (const Object *) this);
}

MainLoop *Engine::get_main_loop() const {
	return (MainLoop *) ___godot_icall_Object(___mb.mb_get_main_loop, (const Object *) this);
}

int64_t Engine::get_physics_frames() const {
	return ___godot_icall_int(___mb.mb_get_physics_frames, (const Object *) this);
}

real_t Engine::get_physics_interpolation_fraction() const {
	return ___godot_icall_float(___mb.mb_get_physics_interpolation_fraction, (const Object *) this);
}

real_t Engine::get_physics_jitter_fix() const {
	return ___godot_icall_float(___mb.mb_get_physics_jitter_fix, (const Object *) this);
}

Object *Engine::get_singleton(const String name) const {
	return (Object *) ___godot_icall_Object_String(___mb.mb_get_singleton, (const Object *) this, name);
}

int64_t Engine::get_target_fps() const {
	return ___godot_icall_int(___mb.mb_get_target_fps, (const Object *) this);
}

real_t Engine::get_time_scale() {
	return ___godot_icall_float(___mb.mb_get_time_scale, (const Object *) this);
}

Dictionary Engine::get_version_info() const {
	return ___godot_icall_Dictionary(___mb.mb_get_version_info, (const Object *) this);
}

bool Engine::has_singleton(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_singleton, (const Object *) this, name);
}

bool Engine::is_editor_hint() const {
	return ___godot_icall_bool(___mb.mb_is_editor_hint, (const Object *) this);
}

bool Engine::is_in_physics_frame() const {
	return ___godot_icall_bool(___mb.mb_is_in_physics_frame, (const Object *) this);
}

void Engine::set_editor_hint(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_editor_hint, (const Object *) this, enabled);
}

void Engine::set_iterations_per_second(const int64_t iterations_per_second) {
	___godot_icall_void_int(___mb.mb_set_iterations_per_second, (const Object *) this, iterations_per_second);
}

void Engine::set_physics_jitter_fix(const real_t physics_jitter_fix) {
	___godot_icall_void_float(___mb.mb_set_physics_jitter_fix, (const Object *) this, physics_jitter_fix);
}

void Engine::set_target_fps(const int64_t target_fps) {
	___godot_icall_void_int(___mb.mb_set_target_fps, (const Object *) this, target_fps);
}

void Engine::set_time_scale(const real_t time_scale) {
	___godot_icall_void_float(___mb.mb_set_time_scale, (const Object *) this, time_scale);
}

}