#include "ShortCut.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"


namespace godot {


ShortCut::___method_bindings ShortCut::___mb = {};

void ShortCut::___init_method_bindings() {
	___mb.mb_get_as_text = godot::api->godot_method_bind_get_method("ShortCut", "get_as_text");
	___mb.mb_get_shortcut = godot::api->godot_method_bind_get_method("ShortCut", "get_shortcut");
	___mb.mb_is_shortcut = godot::api->godot_method_bind_get_method("ShortCut", "is_shortcut");
	___mb.mb_is_valid = godot::api->godot_method_bind_get_method("ShortCut", "is_valid");
	___mb.mb_set_shortcut = godot::api->godot_method_bind_get_method("ShortCut", "set_shortcut");
}

ShortCut *ShortCut::_new()
{
	return (ShortCut *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ShortCut")());
}
String ShortCut::get_as_text() const {
	return ___godot_icall_String(___mb.mb_get_as_text, (const Object *) this);
}

Ref<InputEvent> ShortCut::get_shortcut() const {
	return Ref<InputEvent>::__internal_constructor(___godot_icall_Object(___mb.mb_get_shortcut, (const Object *) this));
}

bool ShortCut::is_shortcut(const Ref<InputEvent> event) const {
	return ___godot_icall_bool_Object(___mb.mb_is_shortcut, (const Object *) this, event.ptr());
}

bool ShortCut::is_valid() const {
	return ___godot_icall_bool(___mb.mb_is_valid, (const Object *) this);
}

void ShortCut::set_shortcut(const Ref<InputEvent> event) {
	___godot_icall_void_Object(___mb.mb_set_shortcut, (const Object *) this, event.ptr());
}

}