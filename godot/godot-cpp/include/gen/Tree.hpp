#ifndef GODOT_CPP_TREE_HPP
#define GODOT_CPP_TREE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Tree.hpp"

#include "Control.hpp"
namespace godot {

class InputEvent;
class TreeItem;
class Object;

class Tree : public Control {
	struct ___method_bindings {
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__popup_select;
		godot_method_bind *mb__range_click_timeout;
		godot_method_bind *mb__scroll_moved;
		godot_method_bind *mb__text_editor_enter;
		godot_method_bind *mb__text_editor_modal_close;
		godot_method_bind *mb__value_editor_changed;
		godot_method_bind *mb_are_column_titles_visible;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_create_item;
		godot_method_bind *mb_ensure_cursor_is_visible;
		godot_method_bind *mb_get_allow_reselect;
		godot_method_bind *mb_get_allow_rmb_select;
		godot_method_bind *mb_get_column_at_position;
		godot_method_bind *mb_get_column_title;
		godot_method_bind *mb_get_column_width;
		godot_method_bind *mb_get_columns;
		godot_method_bind *mb_get_custom_popup_rect;
		godot_method_bind *mb_get_drop_mode_flags;
		godot_method_bind *mb_get_drop_section_at_position;
		godot_method_bind *mb_get_edited;
		godot_method_bind *mb_get_edited_column;
		godot_method_bind *mb_get_item_area_rect;
		godot_method_bind *mb_get_item_at_position;
		godot_method_bind *mb_get_next_selected;
		godot_method_bind *mb_get_pressed_button;
		godot_method_bind *mb_get_root;
		godot_method_bind *mb_get_scroll;
		godot_method_bind *mb_get_select_mode;
		godot_method_bind *mb_get_selected;
		godot_method_bind *mb_get_selected_column;
		godot_method_bind *mb_is_folding_hidden;
		godot_method_bind *mb_is_root_hidden;
		godot_method_bind *mb_set_allow_reselect;
		godot_method_bind *mb_set_allow_rmb_select;
		godot_method_bind *mb_set_column_expand;
		godot_method_bind *mb_set_column_min_width;
		godot_method_bind *mb_set_column_title;
		godot_method_bind *mb_set_column_titles_visible;
		godot_method_bind *mb_set_columns;
		godot_method_bind *mb_set_drop_mode_flags;
		godot_method_bind *mb_set_hide_folding;
		godot_method_bind *mb_set_hide_root;
		godot_method_bind *mb_set_select_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Tree"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum SelectMode {
		SELECT_SINGLE = 0,
		SELECT_ROW = 1,
		SELECT_MULTI = 2,
	};
	enum DropModeFlags {
		DROP_MODE_DISABLED = 0,
		DROP_MODE_ON_ITEM = 1,
		DROP_MODE_INBETWEEN = 2,
	};

	// constants


	static Tree *_new();

	// methods
	void _gui_input(const Ref<InputEvent> arg0);
	void _popup_select(const int64_t arg0);
	void _range_click_timeout();
	void _scroll_moved(const real_t arg0);
	void _text_editor_enter(const String arg0);
	void _text_editor_modal_close();
	void _value_editor_changed(const real_t arg0);
	bool are_column_titles_visible() const;
	void clear();
	TreeItem *create_item(const Object *parent = nullptr, const int64_t idx = -1);
	void ensure_cursor_is_visible();
	bool get_allow_reselect() const;
	bool get_allow_rmb_select() const;
	int64_t get_column_at_position(const Vector2 position) const;
	String get_column_title(const int64_t column) const;
	int64_t get_column_width(const int64_t column) const;
	int64_t get_columns() const;
	Rect2 get_custom_popup_rect() const;
	int64_t get_drop_mode_flags() const;
	int64_t get_drop_section_at_position(const Vector2 position) const;
	TreeItem *get_edited() const;
	int64_t get_edited_column() const;
	Rect2 get_item_area_rect(const Object *item, const int64_t column = -1) const;
	TreeItem *get_item_at_position(const Vector2 position) const;
	TreeItem *get_next_selected(const Object *from);
	int64_t get_pressed_button() const;
	TreeItem *get_root();
	Vector2 get_scroll() const;
	Tree::SelectMode get_select_mode() const;
	TreeItem *get_selected() const;
	int64_t get_selected_column() const;
	bool is_folding_hidden() const;
	bool is_root_hidden() const;
	void set_allow_reselect(const bool allow);
	void set_allow_rmb_select(const bool allow);
	void set_column_expand(const int64_t column, const bool expand);
	void set_column_min_width(const int64_t column, const int64_t min_width);
	void set_column_title(const int64_t column, const String title);
	void set_column_titles_visible(const bool visible);
	void set_columns(const int64_t amount);
	void set_drop_mode_flags(const int64_t flags);
	void set_hide_folding(const bool hide);
	void set_hide_root(const bool enable);
	void set_select_mode(const int64_t mode);

};

}

#endif