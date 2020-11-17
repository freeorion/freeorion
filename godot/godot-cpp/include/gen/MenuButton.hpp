#ifndef GODOT_CPP_MENUBUTTON_HPP
#define GODOT_CPP_MENUBUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Button.hpp"
namespace godot {

class InputEvent;
class PopupMenu;

class MenuButton : public Button {
	struct ___method_bindings {
		godot_method_bind *mb__get_items;
		godot_method_bind *mb__set_items;
		godot_method_bind *mb__unhandled_key_input;
		godot_method_bind *mb_get_popup;
		godot_method_bind *mb_is_switch_on_hover;
		godot_method_bind *mb_set_disable_shortcuts;
		godot_method_bind *mb_set_switch_on_hover;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "MenuButton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static MenuButton *_new();

	// methods
	Array _get_items() const;
	void _set_items(const Array arg0);
	void _unhandled_key_input(const Ref<InputEvent> arg0);
	PopupMenu *get_popup() const;
	bool is_switch_on_hover();
	void set_disable_shortcuts(const bool disabled);
	void set_switch_on_hover(const bool enable);

};

}

#endif