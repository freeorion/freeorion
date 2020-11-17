#ifndef GODOT_CPP_EDITORPROPERTY_HPP
#define GODOT_CPP_EDITORPROPERTY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Container.hpp"
namespace godot {

class InputEvent;
class Control;
class Object;

class EditorProperty : public Container {
	struct ___method_bindings {
		godot_method_bind *mb__focusable_focused;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb_add_focusable;
		godot_method_bind *mb_emit_changed;
		godot_method_bind *mb_get_edited_object;
		godot_method_bind *mb_get_edited_property;
		godot_method_bind *mb_get_label;
		godot_method_bind *mb_get_tooltip_text;
		godot_method_bind *mb_is_checkable;
		godot_method_bind *mb_is_checked;
		godot_method_bind *mb_is_draw_red;
		godot_method_bind *mb_is_keying;
		godot_method_bind *mb_is_read_only;
		godot_method_bind *mb_set_bottom_editor;
		godot_method_bind *mb_set_checkable;
		godot_method_bind *mb_set_checked;
		godot_method_bind *mb_set_draw_red;
		godot_method_bind *mb_set_keying;
		godot_method_bind *mb_set_label;
		godot_method_bind *mb_set_read_only;
		godot_method_bind *mb_update_property;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorProperty"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _focusable_focused(const int64_t arg0);
	void _gui_input(const Ref<InputEvent> arg0);
	void add_focusable(const Control *control);
	void emit_changed(const String property, const Variant value, const String field = "", const bool changing = false);
	Object *get_edited_object();
	String get_edited_property();
	String get_label() const;
	String get_tooltip_text() const;
	bool is_checkable() const;
	bool is_checked() const;
	bool is_draw_red() const;
	bool is_keying() const;
	bool is_read_only() const;
	void set_bottom_editor(const Control *editor);
	void set_checkable(const bool checkable);
	void set_checked(const bool checked);
	void set_draw_red(const bool draw_red);
	void set_keying(const bool keying);
	void set_label(const String text);
	void set_read_only(const bool read_only);
	void update_property();

};

}

#endif