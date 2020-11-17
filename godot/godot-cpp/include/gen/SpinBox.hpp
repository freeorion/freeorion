#ifndef GODOT_CPP_SPINBOX_HPP
#define GODOT_CPP_SPINBOX_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "LineEdit.hpp"

#include "Range.hpp"
namespace godot {

class InputEvent;
class LineEdit;

class SpinBox : public Range {
	struct ___method_bindings {
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__line_edit_focus_exit;
		godot_method_bind *mb__line_edit_input;
		godot_method_bind *mb__range_click_timeout;
		godot_method_bind *mb__text_entered;
		godot_method_bind *mb_apply;
		godot_method_bind *mb_get_align;
		godot_method_bind *mb_get_line_edit;
		godot_method_bind *mb_get_prefix;
		godot_method_bind *mb_get_suffix;
		godot_method_bind *mb_is_editable;
		godot_method_bind *mb_set_align;
		godot_method_bind *mb_set_editable;
		godot_method_bind *mb_set_prefix;
		godot_method_bind *mb_set_suffix;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SpinBox"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static SpinBox *_new();

	// methods
	void _gui_input(const Ref<InputEvent> arg0);
	void _line_edit_focus_exit();
	void _line_edit_input(const Ref<InputEvent> arg0);
	void _range_click_timeout();
	void _text_entered(const String arg0);
	void apply();
	LineEdit::Align get_align() const;
	LineEdit *get_line_edit();
	String get_prefix() const;
	String get_suffix() const;
	bool is_editable() const;
	void set_align(const int64_t align);
	void set_editable(const bool editable);
	void set_prefix(const String prefix);
	void set_suffix(const String suffix);

};

}

#endif