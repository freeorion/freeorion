#ifndef GODOT_CPP_RICHTEXTLABEL_HPP
#define GODOT_CPP_RICHTEXTLABEL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Control.hpp"
namespace godot {

class InputEvent;
class Texture;
class VScrollBar;
class Font;

class RichTextLabel : public Control {
	struct ___method_bindings {
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__scroll_changed;
		godot_method_bind *mb_add_image;
		godot_method_bind *mb_add_text;
		godot_method_bind *mb_append_bbcode;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_get_bbcode;
		godot_method_bind *mb_get_content_height;
		godot_method_bind *mb_get_effects;
		godot_method_bind *mb_get_line_count;
		godot_method_bind *mb_get_percent_visible;
		godot_method_bind *mb_get_tab_size;
		godot_method_bind *mb_get_text;
		godot_method_bind *mb_get_total_character_count;
		godot_method_bind *mb_get_v_scroll;
		godot_method_bind *mb_get_visible_characters;
		godot_method_bind *mb_get_visible_line_count;
		godot_method_bind *mb_install_effect;
		godot_method_bind *mb_is_meta_underlined;
		godot_method_bind *mb_is_overriding_selected_font_color;
		godot_method_bind *mb_is_scroll_active;
		godot_method_bind *mb_is_scroll_following;
		godot_method_bind *mb_is_selection_enabled;
		godot_method_bind *mb_is_using_bbcode;
		godot_method_bind *mb_newline;
		godot_method_bind *mb_parse_bbcode;
		godot_method_bind *mb_parse_expressions_for_values;
		godot_method_bind *mb_pop;
		godot_method_bind *mb_push_align;
		godot_method_bind *mb_push_bold;
		godot_method_bind *mb_push_bold_italics;
		godot_method_bind *mb_push_cell;
		godot_method_bind *mb_push_color;
		godot_method_bind *mb_push_font;
		godot_method_bind *mb_push_indent;
		godot_method_bind *mb_push_italics;
		godot_method_bind *mb_push_list;
		godot_method_bind *mb_push_meta;
		godot_method_bind *mb_push_mono;
		godot_method_bind *mb_push_normal;
		godot_method_bind *mb_push_strikethrough;
		godot_method_bind *mb_push_table;
		godot_method_bind *mb_push_underline;
		godot_method_bind *mb_remove_line;
		godot_method_bind *mb_scroll_to_line;
		godot_method_bind *mb_set_bbcode;
		godot_method_bind *mb_set_effects;
		godot_method_bind *mb_set_meta_underline;
		godot_method_bind *mb_set_override_selected_font_color;
		godot_method_bind *mb_set_percent_visible;
		godot_method_bind *mb_set_scroll_active;
		godot_method_bind *mb_set_scroll_follow;
		godot_method_bind *mb_set_selection_enabled;
		godot_method_bind *mb_set_tab_size;
		godot_method_bind *mb_set_table_column_expand;
		godot_method_bind *mb_set_text;
		godot_method_bind *mb_set_use_bbcode;
		godot_method_bind *mb_set_visible_characters;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "RichTextLabel"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Align {
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT = 2,
		ALIGN_FILL = 3,
	};
	enum ListType {
		LIST_NUMBERS = 0,
		LIST_LETTERS = 1,
		LIST_DOTS = 2,
	};
	enum ItemType {
		ITEM_FRAME = 0,
		ITEM_TEXT = 1,
		ITEM_IMAGE = 2,
		ITEM_NEWLINE = 3,
		ITEM_FONT = 4,
		ITEM_COLOR = 5,
		ITEM_UNDERLINE = 6,
		ITEM_STRIKETHROUGH = 7,
		ITEM_ALIGN = 8,
		ITEM_INDENT = 9,
		ITEM_LIST = 10,
		ITEM_TABLE = 11,
		ITEM_FADE = 12,
		ITEM_SHAKE = 13,
		ITEM_WAVE = 14,
		ITEM_TORNADO = 15,
		ITEM_RAINBOW = 16,
		ITEM_META = 17,
		ITEM_CUSTOMFX = 18,
	};

	// constants


	static RichTextLabel *_new();

	// methods
	void _gui_input(const Ref<InputEvent> arg0);
	void _scroll_changed(const real_t arg0);
	void add_image(const Ref<Texture> image, const int64_t width = 0, const int64_t height = 0);
	void add_text(const String text);
	Error append_bbcode(const String bbcode);
	void clear();
	String get_bbcode() const;
	int64_t get_content_height();
	Array get_effects();
	int64_t get_line_count() const;
	real_t get_percent_visible() const;
	int64_t get_tab_size() const;
	String get_text();
	int64_t get_total_character_count() const;
	VScrollBar *get_v_scroll();
	int64_t get_visible_characters() const;
	int64_t get_visible_line_count() const;
	void install_effect(const Variant effect);
	bool is_meta_underlined() const;
	bool is_overriding_selected_font_color() const;
	bool is_scroll_active() const;
	bool is_scroll_following() const;
	bool is_selection_enabled() const;
	bool is_using_bbcode() const;
	void newline();
	Error parse_bbcode(const String bbcode);
	Dictionary parse_expressions_for_values(const PoolStringArray expressions);
	void pop();
	void push_align(const int64_t align);
	void push_bold();
	void push_bold_italics();
	void push_cell();
	void push_color(const Color color);
	void push_font(const Ref<Font> font);
	void push_indent(const int64_t level);
	void push_italics();
	void push_list(const int64_t type);
	void push_meta(const Variant data);
	void push_mono();
	void push_normal();
	void push_strikethrough();
	void push_table(const int64_t columns);
	void push_underline();
	bool remove_line(const int64_t line);
	void scroll_to_line(const int64_t line);
	void set_bbcode(const String text);
	void set_effects(const Array effects);
	void set_meta_underline(const bool enable);
	void set_override_selected_font_color(const bool override);
	void set_percent_visible(const real_t percent_visible);
	void set_scroll_active(const bool active);
	void set_scroll_follow(const bool follow);
	void set_selection_enabled(const bool enabled);
	void set_tab_size(const int64_t spaces);
	void set_table_column_expand(const int64_t column, const bool expand, const int64_t ratio);
	void set_text(const String text);
	void set_use_bbcode(const bool enable);
	void set_visible_characters(const int64_t amount);

};

}

#endif