#include "Timer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Timer::___method_bindings Timer::___mb = {};

void Timer::___init_method_bindings() {
	___mb.mb_get_time_left = godot::api->godot_method_bind_get_method("Timer", "get_time_left");
	___mb.mb_get_timer_process_mode = godot::api->godot_method_bind_get_method("Timer", "get_timer_process_mode");
	___mb.mb_get_wait_time = godot::api->godot_method_bind_get_method("Timer", "get_wait_time");
	___mb.mb_has_autostart = godot::api->godot_method_bind_get_method("Timer", "has_autostart");
	___mb.mb_is_one_shot = godot::api->godot_method_bind_get_method("Timer", "is_one_shot");
	___mb.mb_is_paused = godot::api->godot_method_bind_get_method("Timer", "is_paused");
	___mb.mb_is_stopped = godot::api->godot_method_bind_get_method("Timer", "is_stopped");
	___mb.mb_set_autostart = godot::api->godot_method_bind_get_method("Timer", "set_autostart");
	___mb.mb_set_one_shot = godot::api->godot_method_bind_get_method("Timer", "set_one_shot");
	___mb.mb_set_paused = godot::api->godot_method_bind_get_method("Timer", "set_paused");
	___mb.mb_set_timer_process_mode = godot::api->godot_method_bind_get_method("Timer", "set_timer_process_mode");
	___mb.mb_set_wait_time = godot::api->godot_method_bind_get_method("Timer", "set_wait_time");
	___mb.mb_start = godot::api->godot_method_bind_get_method("Timer", "start");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("Timer", "stop");
}

Timer *Timer::_new()
{
	return (Timer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Timer")());
}
real_t Timer::get_time_left() const {
	return ___godot_icall_float(___mb.mb_get_time_left, (const Object *) this);
}

Timer::TimerProcessMode Timer::get_timer_process_mode() const {
	return (Timer::TimerProcessMode) ___godot_icall_int(___mb.mb_get_timer_process_mode, (const Object *) this);
}

real_t Timer::get_wait_time() const {
	return ___godot_icall_float(___mb.mb_get_wait_time, (const Object *) this);
}

bool Timer::has_autostart() const {
	return ___godot_icall_bool(___mb.mb_has_autostart, (const Object *) this);
}

bool Timer::is_one_shot() const {
	return ___godot_icall_bool(___mb.mb_is_one_shot, (const Object *) this);
}

bool Timer::is_paused() const {
	return ___godot_icall_bool(___mb.mb_is_paused, (const Object *) this);
}

bool Timer::is_stopped() const {
	return ___godot_icall_bool(___mb.mb_is_stopped, (const Object *) this);
}

void Timer::set_autostart(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_autostart, (const Object *) this, enable);
}

void Timer::set_one_shot(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_one_shot, (const Object *) this, enable);
}

void Timer::set_paused(const bool paused) {
	___godot_icall_void_bool(___mb.mb_set_paused, (const Object *) this, paused);
}

void Timer::set_timer_process_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_timer_process_mode, (const Object *) this, mode);
}

void Timer::set_wait_time(const real_t time_sec) {
	___godot_icall_void_float(___mb.mb_set_wait_time, (const Object *) this, time_sec);
}

void Timer::start(const real_t time_sec) {
	___godot_icall_void_float(___mb.mb_start, (const Object *) this, time_sec);
}

void Timer::stop() {
	___godot_icall_void(___mb.mb_stop, (const Object *) this);
}

}