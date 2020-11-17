#ifndef GODOT_CPP_WINDOWDIALOG_HPP
#define GODOT_CPP_WINDOWDIALOG_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Popup.hpp"
namespace godot {

class InputEvent;
class TextureButton;

class WindowDialog : public Popup {
	struct ___method_bindings {
		godot_method_bind *mb__closed;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb_get_close_button;
		godot_method_bind *mb_get_resizable;
		godot_method_bind *mb_get_title;
		godot_method_bind *mb_set_resizable;
		godot_method_bind *mb_set_title;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "WindowDialog"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static WindowDialog *_new();

	// methods
	void _closed();
	void _gui_input(const Ref<InputEvent> arg0);
	TextureButton *get_close_button();
	bool get_resizable() const;
	String get_title() const;
	void set_resizable(const bool resizable);
	void set_title(const String title);

};

}

#endif