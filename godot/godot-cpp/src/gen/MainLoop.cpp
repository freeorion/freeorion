#include "MainLoop.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"


namespace godot {


MainLoop::___method_bindings MainLoop::___mb = {};

void MainLoop::___init_method_bindings() {
	___mb.mb__drop_files = godot::api->godot_method_bind_get_method("MainLoop", "_drop_files");
	___mb.mb__finalize = godot::api->godot_method_bind_get_method("MainLoop", "_finalize");
	___mb.mb__global_menu_action = godot::api->godot_method_bind_get_method("MainLoop", "_global_menu_action");
	___mb.mb__idle = godot::api->godot_method_bind_get_method("MainLoop", "_idle");
	___mb.mb__initialize = godot::api->godot_method_bind_get_method("MainLoop", "_initialize");
	___mb.mb__input_event = godot::api->godot_method_bind_get_method("MainLoop", "_input_event");
	___mb.mb__input_text = godot::api->godot_method_bind_get_method("MainLoop", "_input_text");
	___mb.mb__iteration = godot::api->godot_method_bind_get_method("MainLoop", "_iteration");
	___mb.mb_finish = godot::api->godot_method_bind_get_method("MainLoop", "finish");
	___mb.mb_idle = godot::api->godot_method_bind_get_method("MainLoop", "idle");
	___mb.mb_init = godot::api->godot_method_bind_get_method("MainLoop", "init");
	___mb.mb_input_event = godot::api->godot_method_bind_get_method("MainLoop", "input_event");
	___mb.mb_input_text = godot::api->godot_method_bind_get_method("MainLoop", "input_text");
	___mb.mb_iteration = godot::api->godot_method_bind_get_method("MainLoop", "iteration");
}

MainLoop *MainLoop::_new()
{
	return (MainLoop *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MainLoop")());
}
void MainLoop::_drop_files(const PoolStringArray files, const int64_t from_screen) {
	___godot_icall_void_PoolStringArray_int(___mb.mb__drop_files, (const Object *) this, files, from_screen);
}

void MainLoop::_finalize() {
	___godot_icall_void(___mb.mb__finalize, (const Object *) this);
}

void MainLoop::_global_menu_action(const Variant id, const Variant meta) {
	___godot_icall_void_Variant_Variant(___mb.mb__global_menu_action, (const Object *) this, id, meta);
}

bool MainLoop::_idle(const real_t delta) {
	return ___godot_icall_bool_float(___mb.mb__idle, (const Object *) this, delta);
}

void MainLoop::_initialize() {
	___godot_icall_void(___mb.mb__initialize, (const Object *) this);
}

void MainLoop::_input_event(const Ref<InputEvent> event) {
	___godot_icall_void_Object(___mb.mb__input_event, (const Object *) this, event.ptr());
}

void MainLoop::_input_text(const String text) {
	___godot_icall_void_String(___mb.mb__input_text, (const Object *) this, text);
}

bool MainLoop::_iteration(const real_t delta) {
	return ___godot_icall_bool_float(___mb.mb__iteration, (const Object *) this, delta);
}

void MainLoop::finish() {
	___godot_icall_void(___mb.mb_finish, (const Object *) this);
}

bool MainLoop::idle(const real_t delta) {
	return ___godot_icall_bool_float(___mb.mb_idle, (const Object *) this, delta);
}

void MainLoop::init() {
	___godot_icall_void(___mb.mb_init, (const Object *) this);
}

void MainLoop::input_event(const Ref<InputEvent> event) {
	___godot_icall_void_Object(___mb.mb_input_event, (const Object *) this, event.ptr());
}

void MainLoop::input_text(const String text) {
	___godot_icall_void_String(___mb.mb_input_text, (const Object *) this, text);
}

bool MainLoop::iteration(const real_t delta) {
	return ___godot_icall_bool_float(___mb.mb_iteration, (const Object *) this, delta);
}

}