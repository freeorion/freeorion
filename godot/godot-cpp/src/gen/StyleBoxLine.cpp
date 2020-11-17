#include "StyleBoxLine.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


StyleBoxLine::___method_bindings StyleBoxLine::___mb = {};

void StyleBoxLine::___init_method_bindings() {
	___mb.mb_get_color = godot::api->godot_method_bind_get_method("StyleBoxLine", "get_color");
	___mb.mb_get_grow_begin = godot::api->godot_method_bind_get_method("StyleBoxLine", "get_grow_begin");
	___mb.mb_get_grow_end = godot::api->godot_method_bind_get_method("StyleBoxLine", "get_grow_end");
	___mb.mb_get_thickness = godot::api->godot_method_bind_get_method("StyleBoxLine", "get_thickness");
	___mb.mb_is_vertical = godot::api->godot_method_bind_get_method("StyleBoxLine", "is_vertical");
	___mb.mb_set_color = godot::api->godot_method_bind_get_method("StyleBoxLine", "set_color");
	___mb.mb_set_grow_begin = godot::api->godot_method_bind_get_method("StyleBoxLine", "set_grow_begin");
	___mb.mb_set_grow_end = godot::api->godot_method_bind_get_method("StyleBoxLine", "set_grow_end");
	___mb.mb_set_thickness = godot::api->godot_method_bind_get_method("StyleBoxLine", "set_thickness");
	___mb.mb_set_vertical = godot::api->godot_method_bind_get_method("StyleBoxLine", "set_vertical");
}

StyleBoxLine *StyleBoxLine::_new()
{
	return (StyleBoxLine *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"StyleBoxLine")());
}
Color StyleBoxLine::get_color() const {
	return ___godot_icall_Color(___mb.mb_get_color, (const Object *) this);
}

real_t StyleBoxLine::get_grow_begin() const {
	return ___godot_icall_float(___mb.mb_get_grow_begin, (const Object *) this);
}

real_t StyleBoxLine::get_grow_end() const {
	return ___godot_icall_float(___mb.mb_get_grow_end, (const Object *) this);
}

int64_t StyleBoxLine::get_thickness() const {
	return ___godot_icall_int(___mb.mb_get_thickness, (const Object *) this);
}

bool StyleBoxLine::is_vertical() const {
	return ___godot_icall_bool(___mb.mb_is_vertical, (const Object *) this);
}

void StyleBoxLine::set_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_color, (const Object *) this, color);
}

void StyleBoxLine::set_grow_begin(const real_t offset) {
	___godot_icall_void_float(___mb.mb_set_grow_begin, (const Object *) this, offset);
}

void StyleBoxLine::set_grow_end(const real_t offset) {
	___godot_icall_void_float(___mb.mb_set_grow_end, (const Object *) this, offset);
}

void StyleBoxLine::set_thickness(const int64_t thickness) {
	___godot_icall_void_int(___mb.mb_set_thickness, (const Object *) this, thickness);
}

void StyleBoxLine::set_vertical(const bool vertical) {
	___godot_icall_void_bool(___mb.mb_set_vertical, (const Object *) this, vertical);
}

}