#include "LinkButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


LinkButton::___method_bindings LinkButton::___mb = {};

void LinkButton::___init_method_bindings() {
	___mb.mb_get_text = godot::api->godot_method_bind_get_method("LinkButton", "get_text");
	___mb.mb_get_underline_mode = godot::api->godot_method_bind_get_method("LinkButton", "get_underline_mode");
	___mb.mb_set_text = godot::api->godot_method_bind_get_method("LinkButton", "set_text");
	___mb.mb_set_underline_mode = godot::api->godot_method_bind_get_method("LinkButton", "set_underline_mode");
}

LinkButton *LinkButton::_new()
{
	return (LinkButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"LinkButton")());
}
String LinkButton::get_text() const {
	return ___godot_icall_String(___mb.mb_get_text, (const Object *) this);
}

LinkButton::UnderlineMode LinkButton::get_underline_mode() const {
	return (LinkButton::UnderlineMode) ___godot_icall_int(___mb.mb_get_underline_mode, (const Object *) this);
}

void LinkButton::set_text(const String text) {
	___godot_icall_void_String(___mb.mb_set_text, (const Object *) this, text);
}

void LinkButton::set_underline_mode(const int64_t underline_mode) {
	___godot_icall_void_int(___mb.mb_set_underline_mode, (const Object *) this, underline_mode);
}

}