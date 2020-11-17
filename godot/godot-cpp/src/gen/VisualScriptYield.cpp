#include "VisualScriptYield.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptYield::___method_bindings VisualScriptYield::___mb = {};

void VisualScriptYield::___init_method_bindings() {
	___mb.mb_get_wait_time = godot::api->godot_method_bind_get_method("VisualScriptYield", "get_wait_time");
	___mb.mb_get_yield_mode = godot::api->godot_method_bind_get_method("VisualScriptYield", "get_yield_mode");
	___mb.mb_set_wait_time = godot::api->godot_method_bind_get_method("VisualScriptYield", "set_wait_time");
	___mb.mb_set_yield_mode = godot::api->godot_method_bind_get_method("VisualScriptYield", "set_yield_mode");
}

VisualScriptYield *VisualScriptYield::_new()
{
	return (VisualScriptYield *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptYield")());
}
real_t VisualScriptYield::get_wait_time() {
	return ___godot_icall_float(___mb.mb_get_wait_time, (const Object *) this);
}

VisualScriptYield::YieldMode VisualScriptYield::get_yield_mode() {
	return (VisualScriptYield::YieldMode) ___godot_icall_int(___mb.mb_get_yield_mode, (const Object *) this);
}

void VisualScriptYield::set_wait_time(const real_t sec) {
	___godot_icall_void_float(___mb.mb_set_wait_time, (const Object *) this, sec);
}

void VisualScriptYield::set_yield_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_yield_mode, (const Object *) this, mode);
}

}