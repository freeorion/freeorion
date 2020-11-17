#ifndef GODOT_CPP_ACCEPTDIALOG_HPP
#define GODOT_CPP_ACCEPTDIALOG_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "WindowDialog.hpp"
namespace godot {

class Button;
class Label;
class Node;

class AcceptDialog : public WindowDialog {
	struct ___method_bindings {
		godot_method_bind *mb__builtin_text_entered;
		godot_method_bind *mb__custom_action;
		godot_method_bind *mb__ok;
		godot_method_bind *mb_add_button;
		godot_method_bind *mb_add_cancel;
		godot_method_bind *mb_get_hide_on_ok;
		godot_method_bind *mb_get_label;
		godot_method_bind *mb_get_ok;
		godot_method_bind *mb_get_text;
		godot_method_bind *mb_has_autowrap;
		godot_method_bind *mb_register_text_enter;
		godot_method_bind *mb_set_autowrap;
		godot_method_bind *mb_set_hide_on_ok;
		godot_method_bind *mb_set_text;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AcceptDialog"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AcceptDialog *_new();

	// methods
	void _builtin_text_entered(const String arg0);
	void _custom_action(const String arg0);
	void _ok();
	Button *add_button(const String text, const bool right = false, const String action = "");
	Button *add_cancel(const String name);
	bool get_hide_on_ok() const;
	Label *get_label();
	Button *get_ok();
	String get_text() const;
	bool has_autowrap();
	void register_text_enter(const Node *line_edit);
	void set_autowrap(const bool autowrap);
	void set_hide_on_ok(const bool enabled);
	void set_text(const String text);

};

}

#endif