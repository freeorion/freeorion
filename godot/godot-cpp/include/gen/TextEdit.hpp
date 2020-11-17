#ifndef GODOT_CPP_TEXTEDIT_HPP
#define GODOT_CPP_TEXTEDIT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Control.hpp"
namespace godot {

class InputEvent;
class PopupMenu;

class TextEdit : public Control {
	struct ___method_bindings {
		godot_method_bind *mb__click_selection_held;
		godot_method_bind *mb__cursor_changed_emit;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__push_current_op;
		godot_method_bind *mb__scroll_moved;
		godot_method_bind *mb__text_changed_emit;
		godot_method_bind *mb__toggle_draw_caret;
		godot_method_bind *mb__update_wrap_at;
		godot_method_bind *mb__v_scroll_input;
		godot_method_bind *mb_add_color_region;
		godot_method_bind *mb_add_keyword_color;
		godot_method_bind *mb_can_fold;
		godot_method_bind *mb_center_viewport_to_cursor;
		godot_method_bind *mb_clear_colors;
		godot_method_bind *mb_clear_undo_history;
		godot_method_bind *mb_copy;
		godot_method_bind *mb_cursor_get_blink_enabled;
		godot_method_bind *mb_cursor_get_blink_speed;
		godot_method_bind *mb_cursor_get_column;
		godot_method_bind *mb_cursor_get_line;
		godot_method_bind *mb_cursor_is_block_mode;
		godot_method_bind *mb_cursor_set_blink_enabled;
		godot_method_bind *mb_cursor_set_blink_speed;
		godot_method_bind *mb_cursor_set_block_mode;
		godot_method_bind *mb_cursor_set_column;
		godot_method_bind *mb_cursor_set_line;
		godot_method_bind *mb_cut;
		godot_method_bind *mb_deselect;
		godot_method_bind *mb_draw_minimap;
		godot_method_bind *mb_fold_all_lines;
		godot_method_bind *mb_fold_line;
		godot_method_bind *mb_get_breakpoints;
		godot_method_bind *mb_get_h_scroll;
		godot_method_bind *mb_get_keyword_color;
		godot_method_bind *mb_get_line;
		godot_method_bind *mb_get_line_count;
		godot_method_bind *mb_get_menu;
		godot_method_bind *mb_get_minimap_width;
		godot_method_bind *mb_get_selection_from_column;
		godot_method_bind *mb_get_selection_from_line;
		godot_method_bind *mb_get_selection_text;
		godot_method_bind *mb_get_selection_to_column;
		godot_method_bind *mb_get_selection_to_line;
		godot_method_bind *mb_get_text;
		godot_method_bind *mb_get_v_scroll;
		godot_method_bind *mb_get_v_scroll_speed;
		godot_method_bind *mb_get_word_under_cursor;
		godot_method_bind *mb_has_keyword_color;
		godot_method_bind *mb_insert_text_at_cursor;
		godot_method_bind *mb_is_breakpoint_gutter_enabled;
		godot_method_bind *mb_is_context_menu_enabled;
		godot_method_bind *mb_is_drawing_fold_gutter;
		godot_method_bind *mb_is_drawing_minimap;
		godot_method_bind *mb_is_drawing_spaces;
		godot_method_bind *mb_is_drawing_tabs;
		godot_method_bind *mb_is_folded;
		godot_method_bind *mb_is_hiding_enabled;
		godot_method_bind *mb_is_highlight_all_occurrences_enabled;
		godot_method_bind *mb_is_highlight_current_line_enabled;
		godot_method_bind *mb_is_line_hidden;
		godot_method_bind *mb_is_overriding_selected_font_color;
		godot_method_bind *mb_is_readonly;
		godot_method_bind *mb_is_right_click_moving_caret;
		godot_method_bind *mb_is_selecting_enabled;
		godot_method_bind *mb_is_selection_active;
		godot_method_bind *mb_is_shortcut_keys_enabled;
		godot_method_bind *mb_is_show_line_numbers_enabled;
		godot_method_bind *mb_is_smooth_scroll_enabled;
		godot_method_bind *mb_is_syntax_coloring_enabled;
		godot_method_bind *mb_is_wrap_enabled;
		godot_method_bind *mb_menu_option;
		godot_method_bind *mb_paste;
		godot_method_bind *mb_redo;
		godot_method_bind *mb_remove_breakpoints;
		godot_method_bind *mb_search;
		godot_method_bind *mb_select;
		godot_method_bind *mb_select_all;
		godot_method_bind *mb_set_breakpoint_gutter_enabled;
		godot_method_bind *mb_set_context_menu_enabled;
		godot_method_bind *mb_set_draw_fold_gutter;
		godot_method_bind *mb_set_draw_spaces;
		godot_method_bind *mb_set_draw_tabs;
		godot_method_bind *mb_set_h_scroll;
		godot_method_bind *mb_set_hiding_enabled;
		godot_method_bind *mb_set_highlight_all_occurrences;
		godot_method_bind *mb_set_highlight_current_line;
		godot_method_bind *mb_set_line_as_hidden;
		godot_method_bind *mb_set_minimap_width;
		godot_method_bind *mb_set_override_selected_font_color;
		godot_method_bind *mb_set_readonly;
		godot_method_bind *mb_set_right_click_moves_caret;
		godot_method_bind *mb_set_selecting_enabled;
		godot_method_bind *mb_set_shortcut_keys_enabled;
		godot_method_bind *mb_set_show_line_numbers;
		godot_method_bind *mb_set_smooth_scroll_enable;
		godot_method_bind *mb_set_syntax_coloring;
		godot_method_bind *mb_set_text;
		godot_method_bind *mb_set_v_scroll;
		godot_method_bind *mb_set_v_scroll_speed;
		godot_method_bind *mb_set_wrap_enabled;
		godot_method_bind *mb_toggle_fold_line;
		godot_method_bind *mb_undo;
		godot_method_bind *mb_unfold_line;
		godot_method_bind *mb_unhide_all_lines;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "TextEdit"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum SearchFlags {
		SEARCH_MATCH_CASE = 1,
		SEARCH_WHOLE_WORDS = 2,
		SEARCH_BACKWARDS = 4,
	};
	enum SearchResult {
		SEARCH_RESULT_COLUMN = 0,
		SEARCH_RESULT_LINE = 1,
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


	static TextEdit *_new();

	// methods
	void _click_selection_held();
	void _cursor_changed_emit();
	void _gui_input(const Ref<InputEvent> arg0);
	void _push_current_op();
	void _scroll_moved(const real_t arg0);
	void _text_changed_emit();
	void _toggle_draw_caret();
	void _update_wrap_at();
	void _v_scroll_input();
	void add_color_region(const String begin_key, const String end_key, const Color color, const bool line_only = false);
	void add_keyword_color(const String keyword, const Color color);
	bool can_fold(const int64_t line) const;
	void center_viewport_to_cursor();
	void clear_colors();
	void clear_undo_history();
	void copy();
	bool cursor_get_blink_enabled() const;
	real_t cursor_get_blink_speed() const;
	int64_t cursor_get_column() const;
	int64_t cursor_get_line() const;
	bool cursor_is_block_mode() const;
	void cursor_set_blink_enabled(const bool enable);
	void cursor_set_blink_speed(const real_t blink_speed);
	void cursor_set_block_mode(const bool enable);
	void cursor_set_column(const int64_t column, const bool adjust_viewport = true);
	void cursor_set_line(const int64_t line, const bool adjust_viewport = true, const bool can_be_hidden = true, const int64_t wrap_index = 0);
	void cut();
	void deselect();
	void draw_minimap(const bool draw);
	void fold_all_lines();
	void fold_line(const int64_t line);
	Array get_breakpoints() const;
	int64_t get_h_scroll() const;
	Color get_keyword_color(const String keyword) const;
	String get_line(const int64_t line) const;
	int64_t get_line_count() const;
	PopupMenu *get_menu() const;
	int64_t get_minimap_width() const;
	int64_t get_selection_from_column() const;
	int64_t get_selection_from_line() const;
	String get_selection_text() const;
	int64_t get_selection_to_column() const;
	int64_t get_selection_to_line() const;
	String get_text();
	real_t get_v_scroll() const;
	real_t get_v_scroll_speed() const;
	String get_word_under_cursor() const;
	bool has_keyword_color(const String keyword) const;
	void insert_text_at_cursor(const String text);
	bool is_breakpoint_gutter_enabled() const;
	bool is_context_menu_enabled();
	bool is_drawing_fold_gutter() const;
	bool is_drawing_minimap() const;
	bool is_drawing_spaces() const;
	bool is_drawing_tabs() const;
	bool is_folded(const int64_t line) const;
	bool is_hiding_enabled() const;
	bool is_highlight_all_occurrences_enabled() const;
	bool is_highlight_current_line_enabled() const;
	bool is_line_hidden(const int64_t line) const;
	bool is_overriding_selected_font_color() const;
	bool is_readonly() const;
	bool is_right_click_moving_caret() const;
	bool is_selecting_enabled() const;
	bool is_selection_active() const;
	bool is_shortcut_keys_enabled() const;
	bool is_show_line_numbers_enabled() const;
	bool is_smooth_scroll_enabled() const;
	bool is_syntax_coloring_enabled() const;
	bool is_wrap_enabled() const;
	void menu_option(const int64_t option);
	void paste();
	void redo();
	void remove_breakpoints();
	PoolIntArray search(const String key, const int64_t flags, const int64_t from_line, const int64_t from_column) const;
	void select(const int64_t from_line, const int64_t from_column, const int64_t to_line, const int64_t to_column);
	void select_all();
	void set_breakpoint_gutter_enabled(const bool enable);
	void set_context_menu_enabled(const bool enable);
	void set_draw_fold_gutter(const bool arg0);
	void set_draw_spaces(const bool arg0);
	void set_draw_tabs(const bool arg0);
	void set_h_scroll(const int64_t value);
	void set_hiding_enabled(const bool enable);
	void set_highlight_all_occurrences(const bool enable);
	void set_highlight_current_line(const bool enabled);
	void set_line_as_hidden(const int64_t line, const bool enable);
	void set_minimap_width(const int64_t width);
	void set_override_selected_font_color(const bool override);
	void set_readonly(const bool enable);
	void set_right_click_moves_caret(const bool enable);
	void set_selecting_enabled(const bool enable);
	void set_shortcut_keys_enabled(const bool enable);
	void set_show_line_numbers(const bool enable);
	void set_smooth_scroll_enable(const bool enable);
	void set_syntax_coloring(const bool enable);
	void set_text(const String text);
	void set_v_scroll(const real_t value);
	void set_v_scroll_speed(const real_t speed);
	void set_wrap_enabled(const bool enable);
	void toggle_fold_line(const int64_t line);
	void undo();
	void unfold_line(const int64_t line);
	void unhide_all_lines();

};

}

#endif