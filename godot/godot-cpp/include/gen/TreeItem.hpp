#ifndef GODOT_CPP_TREEITEM_HPP
#define GODOT_CPP_TREEITEM_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "TreeItem.hpp"

#include "Object.hpp"
namespace godot {

class Texture;
class TreeItem;
class Object;

class TreeItem : public Object {
	struct ___method_bindings {
		godot_method_bind *mb_add_button;
		godot_method_bind *mb_call_recursive;
		godot_method_bind *mb_clear_custom_bg_color;
		godot_method_bind *mb_clear_custom_color;
		godot_method_bind *mb_deselect;
		godot_method_bind *mb_erase_button;
		godot_method_bind *mb_get_button;
		godot_method_bind *mb_get_button_count;
		godot_method_bind *mb_get_button_tooltip;
		godot_method_bind *mb_get_cell_mode;
		godot_method_bind *mb_get_children;
		godot_method_bind *mb_get_custom_bg_color;
		godot_method_bind *mb_get_custom_color;
		godot_method_bind *mb_get_custom_minimum_height;
		godot_method_bind *mb_get_expand_right;
		godot_method_bind *mb_get_icon;
		godot_method_bind *mb_get_icon_max_width;
		godot_method_bind *mb_get_icon_modulate;
		godot_method_bind *mb_get_icon_region;
		godot_method_bind *mb_get_metadata;
		godot_method_bind *mb_get_next;
		godot_method_bind *mb_get_next_visible;
		godot_method_bind *mb_get_parent;
		godot_method_bind *mb_get_prev;
		godot_method_bind *mb_get_prev_visible;
		godot_method_bind *mb_get_range;
		godot_method_bind *mb_get_range_config;
		godot_method_bind *mb_get_text;
		godot_method_bind *mb_get_text_align;
		godot_method_bind *mb_get_tooltip;
		godot_method_bind *mb_is_button_disabled;
		godot_method_bind *mb_is_checked;
		godot_method_bind *mb_is_collapsed;
		godot_method_bind *mb_is_custom_set_as_button;
		godot_method_bind *mb_is_editable;
		godot_method_bind *mb_is_folding_disabled;
		godot_method_bind *mb_is_selectable;
		godot_method_bind *mb_is_selected;
		godot_method_bind *mb_move_to_bottom;
		godot_method_bind *mb_move_to_top;
		godot_method_bind *mb_remove_child;
		godot_method_bind *mb_select;
		godot_method_bind *mb_set_button;
		godot_method_bind *mb_set_button_disabled;
		godot_method_bind *mb_set_cell_mode;
		godot_method_bind *mb_set_checked;
		godot_method_bind *mb_set_collapsed;
		godot_method_bind *mb_set_custom_as_button;
		godot_method_bind *mb_set_custom_bg_color;
		godot_method_bind *mb_set_custom_color;
		godot_method_bind *mb_set_custom_draw;
		godot_method_bind *mb_set_custom_minimum_height;
		godot_method_bind *mb_set_disable_folding;
		godot_method_bind *mb_set_editable;
		godot_method_bind *mb_set_expand_right;
		godot_method_bind *mb_set_icon;
		godot_method_bind *mb_set_icon_max_width;
		godot_method_bind *mb_set_icon_modulate;
		godot_method_bind *mb_set_icon_region;
		godot_method_bind *mb_set_metadata;
		godot_method_bind *mb_set_range;
		godot_method_bind *mb_set_range_config;
		godot_method_bind *mb_set_selectable;
		godot_method_bind *mb_set_text;
		godot_method_bind *mb_set_text_align;
		godot_method_bind *mb_set_tooltip;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "TreeItem"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TreeCellMode {
		CELL_MODE_STRING = 0,
		CELL_MODE_CHECK = 1,
		CELL_MODE_RANGE = 2,
		CELL_MODE_ICON = 3,
		CELL_MODE_CUSTOM = 4,
	};
	enum TextAlign {
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT = 2,
	};

	// constants

	// methods
	void add_button(const int64_t column, const Ref<Texture> button, const int64_t button_idx = -1, const bool disabled = false, const String tooltip = "");
	Variant call_recursive(const String method, const Array& __var_args = Array());
	void clear_custom_bg_color(const int64_t column);
	void clear_custom_color(const int64_t column);
	void deselect(const int64_t column);
	void erase_button(const int64_t column, const int64_t button_idx);
	Ref<Texture> get_button(const int64_t column, const int64_t button_idx) const;
	int64_t get_button_count(const int64_t column) const;
	String get_button_tooltip(const int64_t column, const int64_t button_idx) const;
	TreeItem::TreeCellMode get_cell_mode(const int64_t column) const;
	TreeItem *get_children();
	Color get_custom_bg_color(const int64_t column) const;
	Color get_custom_color(const int64_t column) const;
	int64_t get_custom_minimum_height() const;
	bool get_expand_right(const int64_t column) const;
	Ref<Texture> get_icon(const int64_t column) const;
	int64_t get_icon_max_width(const int64_t column) const;
	Color get_icon_modulate(const int64_t column) const;
	Rect2 get_icon_region(const int64_t column) const;
	Variant get_metadata(const int64_t column) const;
	TreeItem *get_next();
	TreeItem *get_next_visible(const bool wrap = false);
	TreeItem *get_parent();
	TreeItem *get_prev();
	TreeItem *get_prev_visible(const bool wrap = false);
	real_t get_range(const int64_t column) const;
	Dictionary get_range_config(const int64_t column);
	String get_text(const int64_t column) const;
	TreeItem::TextAlign get_text_align(const int64_t column) const;
	String get_tooltip(const int64_t column) const;
	bool is_button_disabled(const int64_t column, const int64_t button_idx) const;
	bool is_checked(const int64_t column) const;
	bool is_collapsed();
	bool is_custom_set_as_button(const int64_t column) const;
	bool is_editable(const int64_t column);
	bool is_folding_disabled() const;
	bool is_selectable(const int64_t column) const;
	bool is_selected(const int64_t column);
	void move_to_bottom();
	void move_to_top();
	void remove_child(const Object *child);
	void select(const int64_t column);
	void set_button(const int64_t column, const int64_t button_idx, const Ref<Texture> button);
	void set_button_disabled(const int64_t column, const int64_t button_idx, const bool disabled);
	void set_cell_mode(const int64_t column, const int64_t mode);
	void set_checked(const int64_t column, const bool checked);
	void set_collapsed(const bool enable);
	void set_custom_as_button(const int64_t column, const bool enable);
	void set_custom_bg_color(const int64_t column, const Color color, const bool just_outline = false);
	void set_custom_color(const int64_t column, const Color color);
	void set_custom_draw(const int64_t column, const Object *object, const String callback);
	void set_custom_minimum_height(const int64_t height);
	void set_disable_folding(const bool disable);
	void set_editable(const int64_t column, const bool enabled);
	void set_expand_right(const int64_t column, const bool enable);
	void set_icon(const int64_t column, const Ref<Texture> texture);
	void set_icon_max_width(const int64_t column, const int64_t width);
	void set_icon_modulate(const int64_t column, const Color modulate);
	void set_icon_region(const int64_t column, const Rect2 region);
	void set_metadata(const int64_t column, const Variant meta);
	void set_range(const int64_t column, const real_t value);
	void set_range_config(const int64_t column, const real_t min, const real_t max, const real_t step, const bool expr = false);
	void set_selectable(const int64_t column, const bool selectable);
	void set_text(const int64_t column, const String text);
	void set_text_align(const int64_t column, const int64_t text_align);
	void set_tooltip(const int64_t column, const String tooltip);
	template <class... Args> Variant call_recursive(const String method, Args... args){
		return call_recursive(method, Array::make(args...));
	}

};

}

#endif