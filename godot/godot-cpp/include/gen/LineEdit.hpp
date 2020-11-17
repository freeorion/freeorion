#ifndef GODOT_CPP_LINEEDIT_HPP
#define GODOT_CPP_LINEEDIT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "LineEdit.hpp"

#include "Control.hpp"
namespace godot {

class InputEvent;
class PopupMenu;
class Texture;

class LineEdit : public Control {
	struct ___method_bindings {
		godot_method_bind *mb__editor_settings_changed;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__text_changed;
		godot_method_bind *mb__toggle_draw_caret;
		godot_method_bind *mb_append_at_cursor;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_cursor_get_blink_enabled;
		godot_method_bind *mb_cursor_get_blink_speed;
		godot_method_bind *mb_cursor_set_blink_enabled;
		godot_method_bind *mb_cursor_set_blink_speed;
		godot_method_bind *mb_deselect;
		godot_method_bind *mb_get_align;
		godot_method_bind *mb_get_cursor_position;
		godot_method_bind *mb_get_expand_to_text_length;
		godot_method_bind *mb_get_max_length;
		godot_method_bind *mb_get_menu;
		godot_method_bind *mb_get_placeholder;
		godot_method_bind *mb_get_placeholder_alpha;
		godot_method_bind *mb_get_right_icon;
		godot_method_bind *mb_get_secret_character;
		godot_method_bind *mb_get_text;
		godot_method_bind *mb_is_clear_button_enabled;
		godot_method_bind *mb_is_context_menu_enabled;
		godot_method_bind *mb_is_editable;
		godot_method_bind *mb_is_secret;
		godot_method_bind *mb_is_selecting_enabled;
		godot_method_bind *mb_is_shortcut_keys_enabled;
		godot_method_bind *mb_menu_option;
		godot_method_bind *mb_select;
		godot_method_bind *mb_select_all;
		godot_method_bind *mb_set_align;
		godot_method_bind *mb_set_clear_button_enabled;
		godot_method_bind *mb_set_context_menu_enabled;
		godot_method_bind *mb_set_cursor_position;
		godot_method_bind *mb_set_editable;
		godot_method_bind *mb_set_expand_to_text_length;
		godot_method_bind *mb_set_max_length;
		godot_method_bind *mb_set_placeholder;
		godot_method_bind *mb_set_placeholder_alpha;
		godot_method_bind *mb_set_right_icon;
		godot_method_bind *mb_set_secret;
		godot_method_bind *mb_set_secret_character;
		godot_method_bind *mb_set_selecting_enabled;
		godot_method_bind *mb_set_shortcut_keys_enabled;
		godot_method_bind *mb_set_text;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "LineEdit"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Align {
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT = 2,
		ALIGN_FILL = 3,
	};
	enum MenuItems {
		MENU_CUT = 0,
		MENU_COPY = 1,
		MENU_PASTE = 2,
		MENU_CLEAR = 3,
		MENU_SELECT_ALL = 4,
		MENU_UNDO = 5,
		MENU_REDO = 6,
		MENU_MAX = 7,
	};

	// constants


	static LineEdit *_new();

	// methods
	void _editor_settings_changed();
	void _gui_input(const Ref<InputEvent> arg0);
	void _text_changed();
	void _toggle_draw_caret();
	void append_at_cursor(const String text);
	void clear();
	bool cursor_get_blink_enabled() const;
	real_t cursor_get_blink_speed() const;
	void cursor_set_blink_enabled(const bool enabled);
	void cursor_set_blink_speed(const real_t blink_speed);
	void deselect();
	LineEdit::Align get_align() const;
	int64_t get_cursor_position() const;
	bool get_expand_to_text_length() const;
	int64_t get_max_length() const;
	PopupMenu *get_menu() const;
	String get_placeholder() const;
	real_t get_placeholder_alpha() const;
	Ref<Texture> get_right_icon();
	String get_secret_character() const;
	String get_text() const;
	bool is_clear_button_enabled() const;
	bool is_context_menu_enabled();
	bool is_editable() const;
	bool is_secret() const;
	bool is_selecting_enabled() const;
	bool is_shortcut_keys_enabled() const;
	void menu_option(const int64_t option);
	void select(const int64_t from = 0, const int64_t to = -1);
	void select_all();
	void set_align(const int64_t align);
	void set_clear_button_enabled(const bool enable);
	void set_context_menu_enabled(const bool enable);
	void set_cursor_position(const int64_t position);
	void set_editable(const bool enabled);
	void set_expand_to_text_length(const bool enabled);
	void set_max_length(const int64_t chars);
	void set_placeholder(const String text);
	void set_placeholder_alpha(const real_t alpha);
	void set_right_icon(const Ref<Texture> icon);
	void set_secret(const bool enabled);
	void set_secret_character(const String character);
	void set_selecting_enabled(const bool enable);
	void set_shortcut_keys_enabled(const bool enable);
	void set_text(const String text);

};

}

#endif