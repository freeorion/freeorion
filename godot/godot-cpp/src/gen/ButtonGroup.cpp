#include "ButtonGroup.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "BaseButton.hpp"


namespace godot {


ButtonGroup::___method_bindings ButtonGroup::___mb = {};

void ButtonGroup::___init_method_bindings() {
	___mb.mb_get_buttons = godot::api->godot_method_bind_get_method("ButtonGroup", "get_buttons");
	___mb.mb_get_pressed_button = godot::api->godot_method_bind_get_method("ButtonGroup", "get_pressed_button");
}

ButtonGroup *ButtonGroup::_new()
{
	return (ButtonGroup *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ButtonGroup")());
}
Array ButtonGroup::get_buttons() {
	return ___godot_icall_Array(___mb.mb_get_buttons, (const Object *) this);
}

BaseButton *ButtonGroup::get_pressed_button() {
	return (BaseButton *) ___godot_icall_Object(___mb.mb_get_pressed_button, (const Object *) this);
}

}