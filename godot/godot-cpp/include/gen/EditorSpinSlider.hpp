#ifndef GODOT_CPP_EDITORSPINSLIDER_HPP
#define GODOT_CPP_EDITORSPINSLIDER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Range.hpp"
namespace godot {

class InputEvent;

class EditorSpinSlider : public Range {
	struct ___method_bindings {
		godot_method_bind *mb__grabber_gui_input;
		godot_method_bind *mb__grabber_mouse_entered;
		godot_method_bind *mb__grabber_mouse_exited;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__value_focus_exited;
		godot_method_bind *mb__value_input_closed;
		godot_method_bind *mb__value_input_entered;
		godot_method_bind *mb_get_label;
		godot_method_bind *mb_is_flat;
		godot_method_bind *mb_is_read_only;
		godot_method_bind *mb_set_flat;
		godot_method_bind *mb_set_label;
		godot_method_bind *mb_set_read_only;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorSpinSlider"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _grabber_gui_input(const Ref<InputEvent> arg0);
	void _grabber_mouse_entered();
	void _grabber_mouse_exited();
	void _gui_input(const Ref<InputEvent> arg0);
	void _value_focus_exited();
	void _value_input_closed();
	void _value_input_entered(const String arg0);
	String get_label() const;
	bool is_flat() const;
	bool is_read_only() const;
	void set_flat(const bool flat);
	void set_label(const String label);
	void set_read_only(const bool read_only);

};

}

#endif