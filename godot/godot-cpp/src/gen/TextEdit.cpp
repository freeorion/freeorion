#include "TextEdit.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "PopupMenu.hpp"


namespace godot {


TextEdit::___method_bindings TextEdit::___mb = {};

void TextEdit::___init_method_bindings() {
	___mb.mb__click_selection_held = godot::api->godot_method_bind_get_method("TextEdit", "_click_selection_held");
	___mb.mb__cursor_changed_emit = godot::api->godot_method_bind_get_method("TextEdit", "_cursor_changed_emit");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("TextEdit", "_gui_input");
	___mb.mb__push_current_op = godot::api->godot_method_bind_get_method("TextEdit", "_push_current_op");
	___mb.mb__scroll_moved = godot::api->godot_method_bind_get_method("TextEdit", "_scroll_moved");
	___mb.mb__text_changed_emit = godot::api->godot_method_bind_get_method("TextEdit", "_text_changed_emit");
	___mb.mb__toggle_draw_caret = godot::api->godot_method_bind_get_method("TextEdit", "_toggle_draw_caret");
	___mb.mb__update_wrap_at = godot::api->godot_method_bind_get_method("TextEdit", "_update_wrap_at");
	___mb.mb__v_scroll_input = godot::api->godot_method_bind_get_method("TextEdit", "_v_scroll_input");
	___mb.mb_add_color_region = godot::api->godot_method_bind_get_method("TextEdit", "add_color_region");
	___mb.mb_add_keyword_color = godot::api->godot_method_bind_get_method("TextEdit", "add_keyword_color");
	___mb.mb_can_fold = godot::api->godot_method_bind_get_method("TextEdit", "can_fold");
	___mb.mb_center_viewport_to_cursor = godot::api->godot_method_bind_get_method("TextEdit", "center_viewport_to_cursor");
	___mb.mb_clear_colors = godot::api->godot_method_bind_get_method("TextEdit", "clear_colors");
	___mb.mb_clear_undo_history = godot::api->godot_method_bind_get_method("TextEdit", "clear_undo_history");
	___mb.mb_copy = godot::api->godot_method_bind_get_method("TextEdit", "copy");
	___mb.mb_cursor_get_blink_enabled = godot::api->godot_method_bind_get_method("TextEdit", "cursor_get_blink_enabled");
	___mb.mb_cursor_get_blink_speed = godot::api->godot_method_bind_get_method("TextEdit", "cursor_get_blink_speed");
	___mb.mb_cursor_get_column = godot::api->godot_method_bind_get_method("TextEdit", "cursor_get_column");
	___mb.mb_cursor_get_line = godot::api->godot_method_bind_get_method("TextEdit", "cursor_get_line");
	___mb.mb_cursor_is_block_mode = godot::api->godot_method_bind_get_method("TextEdit", "cursor_is_block_mode");
	___mb.mb_cursor_set_blink_enabled = godot::api->godot_method_bind_get_method("TextEdit", "cursor_set_blink_enabled");
	___mb.mb_cursor_set_blink_speed = godot::api->godot_method_bind_get_method("TextEdit", "cursor_set_blink_speed");
	___mb.mb_cursor_set_block_mode = godot::api->godot_method_bind_get_method("TextEdit", "cursor_set_block_mode");
	___mb.mb_cursor_set_column = godot::api->godot_method_bind_get_method("TextEdit", "cursor_set_column");
	___mb.mb_cursor_set_line = godot::api->godot_method_bind_get_method("TextEdit", "cursor_set_line");
	___mb.mb_cut = godot::api->godot_method_bind_get_method("TextEdit", "cut");
	___mb.mb_deselect = godot::api->godot_method_bind_get_method("TextEdit", "deselect");
	___mb.mb_draw_minimap = godot::api->godot_method_bind_get_method("TextEdit", "draw_minimap");
	___mb.mb_fold_all_lines = godot::api->godot_method_bind_get_method("TextEdit", "fold_all_lines");
	___mb.mb_fold_line = godot::api->godot_method_bind_get_method("TextEdit", "fold_line");
	___mb.mb_get_breakpoints = godot::api->godot_method_bind_get_method("TextEdit", "get_breakpoints");
	___mb.mb_get_h_scroll = godot::api->godot_method_bind_get_method("TextEdit", "get_h_scroll");
	___mb.mb_get_keyword_color = godot::api->godot_method_bind_get_method("TextEdit", "get_keyword_color");
	___mb.mb_get_line = godot::api->godot_method_bind_get_method("TextEdit", "get_line");
	___mb.mb_get_line_count = godot::api->godot_method_bind_get_method("TextEdit", "get_line_count");
	___mb.mb_get_menu = godot::api->godot_method_bind_get_method("TextEdit", "get_menu");
	___mb.mb_get_minimap_width = godot::api->godot_method_bind_get_method("TextEdit", "get_minimap_width");
	___mb.mb_get_selection_from_column = godot::api->godot_method_bind_get_method("TextEdit", "get_selection_from_column");
	___mb.mb_get_selection_from_line = godot::api->godot_method_bind_get_method("TextEdit", "get_selection_from_line");
	___mb.mb_get_selection_text = godot::api->godot_method_bind_get_method("TextEdit", "get_selection_text");
	___mb.mb_get_selection_to_column = godot::api->godot_method_bind_get_method("TextEdit", "get_selection_to_column");
	___mb.mb_get_selection_to_line = godot::api->godot_method_bind_get_method("TextEdit", "get_selection_to_line");
	___mb.mb_get_text = godot::api->godot_method_bind_get_method("TextEdit", "get_text");
	___mb.mb_get_v_scroll = godot::api->godot_method_bind_get_method("TextEdit", "get_v_scroll");
	___mb.mb_get_v_scroll_speed = godot::api->godot_method_bind_get_method("TextEdit", "get_v_scroll_speed");
	___mb.mb_get_word_under_cursor = godot::api->godot_method_bind_get_method("TextEdit", "get_word_under_cursor");
	___mb.mb_has_keyword_color = godot::api->godot_method_bind_get_method("TextEdit", "has_keyword_color");
	___mb.mb_insert_text_at_cursor = godot::api->godot_method_bind_get_method("TextEdit", "insert_text_at_cursor");
	___mb.mb_is_breakpoint_gutter_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_breakpoint_gutter_enabled");
	___mb.mb_is_context_menu_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_context_menu_enabled");
	___mb.mb_is_drawing_fold_gutter = godot::api->godot_method_bind_get_method("TextEdit", "is_drawing_fold_gutter");
	___mb.mb_is_drawing_minimap = godot::api->godot_method_bind_get_method("TextEdit", "is_drawing_minimap");
	___mb.mb_is_drawing_spaces = godot::api->godot_method_bind_get_method("TextEdit", "is_drawing_spaces");
	___mb.mb_is_drawing_tabs = godot::api->godot_method_bind_get_method("TextEdit", "is_drawing_tabs");
	___mb.mb_is_folded = godot::api->godot_method_bind_get_method("TextEdit", "is_folded");
	___mb.mb_is_hiding_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_hiding_enabled");
	___mb.mb_is_highlight_all_occurrences_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_highlight_all_occurrences_enabled");
	___mb.mb_is_highlight_current_line_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_highlight_current_line_enabled");
	___mb.mb_is_line_hidden = godot::api->godot_method_bind_get_method("TextEdit", "is_line_hidden");
	___mb.mb_is_overriding_selected_font_color = godot::api->godot_method_bind_get_method("TextEdit", "is_overriding_selected_font_color");
	___mb.mb_is_readonly = godot::api->godot_method_bind_get_method("TextEdit", "is_readonly");
	___mb.mb_is_right_click_moving_caret = godot::api->godot_method_bind_get_method("TextEdit", "is_right_click_moving_caret");
	___mb.mb_is_selecting_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_selecting_enabled");
	___mb.mb_is_selection_active = godot::api->godot_method_bind_get_method("TextEdit", "is_selection_active");
	___mb.mb_is_shortcut_keys_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_shortcut_keys_enabled");
	___mb.mb_is_show_line_numbers_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_show_line_numbers_enabled");
	___mb.mb_is_smooth_scroll_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_smooth_scroll_enabled");
	___mb.mb_is_syntax_coloring_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_syntax_coloring_enabled");
	___mb.mb_is_wrap_enabled = godot::api->godot_method_bind_get_method("TextEdit", "is_wrap_enabled");
	___mb.mb_menu_option = godot::api->godot_method_bind_get_method("TextEdit", "menu_option");
	___mb.mb_paste = godot::api->godot_method_bind_get_method("TextEdit", "paste");
	___mb.mb_redo = godot::api->godot_method_bind_get_method("TextEdit", "redo");
	___mb.mb_remove_breakpoints = godot::api->godot_method_bind_get_method("TextEdit", "remove_breakpoints");
	___mb.mb_search = godot::api->godot_method_bind_get_method("TextEdit", "search");
	___mb.mb_select = godot::api->godot_method_bind_get_method("TextEdit", "select");
	___mb.mb_select_all = godot::api->godot_method_bind_get_method("TextEdit", "select_all");
	___mb.mb_set_breakpoint_gutter_enabled = godot::api->godot_method_bind_get_method("TextEdit", "set_breakpoint_gutter_enabled");
	___mb.mb_set_context_menu_enabled = godot::api->godot_method_bind_get_method("TextEdit", "set_context_menu_enabled");
	___mb.mb_set_draw_fold_gutter = godot::api->godot_method_bind_get_method("TextEdit", "set_draw_fold_gutter");
	___mb.mb_set_draw_spaces = godot::api->godot_method_bind_get_method("TextEdit", "set_draw_spaces");
	___mb.mb_set_draw_tabs = godot::api->godot_method_bind_get_method("TextEdit", "set_draw_tabs");
	___mb.mb_set_h_scroll = godot::api->godot_method_bind_get_method("TextEdit", "set_h_scroll");
	___mb.mb_set_hiding_enabled = godot::api->godot_method_bind_get_method("TextEdit", "set_hiding_enabled");
	___mb.mb_set_highlight_all_occurrences = godot::api->godot_method_bind_get_method("TextEdit", "set_highlight_all_occurrences");
	___mb.mb_set_highlight_current_line = godot::api->godot_method_bind_get_method("TextEdit", "set_highlight_current_line");
	___mb.mb_set_line_as_hidden = godot::api->godot_method_bind_get_method("TextEdit", "set_line_as_hidden");
	___mb.mb_set_minimap_width = godot::api->godot_method_bind_get_method("TextEdit", "set_minimap_width");
	___mb.mb_set_override_selected_font_color = godot::api->godot_method_bind_get_method("TextEdit", "set_override_selected_font_color");
	___mb.mb_set_readonly = godot::api->godot_method_bind_get_method("TextEdit", "set_readonly");
	___mb.mb_set_right_click_moves_caret = godot::api->godot_method_bind_get_method("TextEdit", "set_right_click_moves_caret");
	___mb.mb_set_selecting_enabled = godot::api->godot_method_bind_get_method("TextEdit", "set_selecting_enabled");
	___mb.mb_set_shortcut_keys_enabled = godot::api->godot_method_bind_get_method("TextEdit", "set_shortcut_keys_enabled");
	___mb.mb_set_show_line_numbers = godot::api->godot_method_bind_get_method("TextEdit", "set_show_line_numbers");
	___mb.mb_set_smooth_scroll_enable = godot::api->godot_method_bind_get_method("TextEdit", "set_smooth_scroll_enable");
	___mb.mb_set_syntax_coloring = godot::api->godot_method_bind_get_method("TextEdit", "set_syntax_coloring");
	___mb.mb_set_text = godot::api->godot_method_bind_get_method("TextEdit", "set_text");
	___mb.mb_set_v_scroll = godot::api->godot_method_bind_get_method("TextEdit", "set_v_scroll");
	___mb.mb_set_v_scroll_speed = godot::api->godot_method_bind_get_method("TextEdit", "set_v_scroll_speed");
	___mb.mb_set_wrap_enabled = godot::api->godot_method_bind_get_method("TextEdit", "set_wrap_enabled");
	___mb.mb_toggle_fold_line = godot::api->godot_method_bind_get_method("TextEdit", "toggle_fold_line");
	___mb.mb_undo = godot::api->godot_method_bind_get_method("TextEdit", "undo");
	___mb.mb_unfold_line = godot::api->godot_method_bind_get_method("TextEdit", "unfold_line");
	___mb.mb_unhide_all_lines = godot::api->godot_method_bind_get_method("TextEdit", "unhide_all_lines");
}

TextEdit *TextEdit::_new()
{
	return (TextEdit *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TextEdit")());
}
void TextEdit::_click_selection_held() {
	___godot_icall_void(___mb.mb__click_selection_held, (const Object *) this);
}

void TextEdit::_cursor_changed_emit() {
	___godot_icall_void(___mb.mb__cursor_changed_emit, (const Object *) this);
}

void TextEdit::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void TextEdit::_push_current_op() {
	___godot_icall_void(___mb.mb__push_current_op, (const Object *) this);
}

void TextEdit::_scroll_moved(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__scroll_moved, (const Object *) this, arg0);
}

void TextEdit::_text_changed_emit() {
	___godot_icall_void(___mb.mb__text_changed_emit, (const Object *) this);
}

void TextEdit::_toggle_draw_caret() {
	___godot_icall_void(___mb.mb__toggle_draw_caret, (const Object *) this);
}

void TextEdit::_update_wrap_at() {
	___godot_icall_void(___mb.mb__update_wrap_at, (const Object *) this);
}

void TextEdit::_v_scroll_input() {
	___godot_icall_void(___mb.mb__v_scroll_input, (const Object *) this);
}

void TextEdit::add_color_region(const String begin_key, const String end_key, const Color color, const bool line_only) {
	___godot_icall_void_String_String_Color_bool(___mb.mb_add_color_region, (const Object *) this, begin_key, end_key, color, line_only);
}

void TextEdit::add_keyword_color(const String keyword, const Color color) {
	___godot_icall_void_String_Color(___mb.mb_add_keyword_color, (const Object *) this, keyword, color);
}

bool TextEdit::can_fold(const int64_t line) const {
	return ___godot_icall_bool_int(___mb.mb_can_fold, (const Object *) this, line);
}

void TextEdit::center_viewport_to_cursor() {
	___godot_icall_void(___mb.mb_center_viewport_to_cursor, (const Object *) this);
}

void TextEdit::clear_colors() {
	___godot_icall_void(___mb.mb_clear_colors, (const Object *) this);
}

void TextEdit::clear_undo_history() {
	___godot_icall_void(___mb.mb_clear_undo_history, (const Object *) this);
}

void TextEdit::copy() {
	___godot_icall_void(___mb.mb_copy, (const Object *) this);
}

bool TextEdit::cursor_get_blink_enabled() const {
	return ___godot_icall_bool(___mb.mb_cursor_get_blink_enabled, (const Object *) this);
}

real_t TextEdit::cursor_get_blink_speed() const {
	return ___godot_icall_float(___mb.mb_cursor_get_blink_speed, (const Object *) this);
}

int64_t TextEdit::cursor_get_column() const {
	return ___godot_icall_int(___mb.mb_cursor_get_column, (const Object *) this);
}

int64_t TextEdit::cursor_get_line() const {
	return ___godot_icall_int(___mb.mb_cursor_get_line, (const Object *) this);
}

bool TextEdit::cursor_is_block_mode() const {
	return ___godot_icall_bool(___mb.mb_cursor_is_block_mode, (const Object *) this);
}

void TextEdit::cursor_set_blink_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_cursor_set_blink_enabled, (const Object *) this, enable);
}

void TextEdit::cursor_set_blink_speed(const real_t blink_speed) {
	___godot_icall_void_float(___mb.mb_cursor_set_blink_speed, (const Object *) this, blink_speed);
}

void TextEdit::cursor_set_block_mode(const bool enable) {
	___godot_icall_void_bool(___mb.mb_cursor_set_block_mode, (const Object *) this, enable);
}

void TextEdit::cursor_set_column(const int64_t column, const bool adjust_viewport) {
	___godot_icall_void_int_bool(___mb.mb_cursor_set_column, (const Object *) this, column, adjust_viewport);
}

void TextEdit::cursor_set_line(const int64_t line, const bool adjust_viewport, const bool can_be_hidden, const int64_t wrap_index) {
	___godot_icall_void_int_bool_bool_int(___mb.mb_cursor_set_line, (const Object *) this, line, adjust_viewport, can_be_hidden, wrap_index);
}

void TextEdit::cut() {
	___godot_icall_void(___mb.mb_cut, (const Object *) this);
}

void TextEdit::deselect() {
	___godot_icall_void(___mb.mb_deselect, (const Object *) this);
}

void TextEdit::draw_minimap(const bool draw) {
	___godot_icall_void_bool(___mb.mb_draw_minimap, (const Object *) this, draw);
}

void TextEdit::fold_all_lines() {
	___godot_icall_void(___mb.mb_fold_all_lines, (const Object *) this);
}

void TextEdit::fold_line(const int64_t line) {
	___godot_icall_void_int(___mb.mb_fold_line, (const Object *) this, line);
}

Array TextEdit::get_breakpoints() const {
	return ___godot_icall_Array(___mb.mb_get_breakpoints, (const Object *) this);
}

int64_t TextEdit::get_h_scroll() const {
	return ___godot_icall_int(___mb.mb_get_h_scroll, (const Object *) this);
}

Color TextEdit::get_keyword_color(const String keyword) const {
	return ___godot_icall_Color_String(___mb.mb_get_keyword_color, (const Object *) this, keyword);
}

String TextEdit::get_line(const int64_t line) const {
	return ___godot_icall_String_int(___mb.mb_get_line, (const Object *) this, line);
}

int64_t TextEdit::get_line_count() const {
	return ___godot_icall_int(___mb.mb_get_line_count, (const Object *) this);
}

PopupMenu *TextEdit::get_menu() const {
	return (PopupMenu *) ___godot_icall_Object(___mb.mb_get_menu, (const Object *) this);
}

int64_t TextEdit::get_minimap_width() const {
	return ___godot_icall_int(___mb.mb_get_minimap_width, (const Object *) this);
}

int64_t TextEdit::get_selection_from_column() const {
	return ___godot_icall_int(___mb.mb_get_selection_from_column, (const Object *) this);
}

int64_t TextEdit::get_selection_from_line() const {
	return ___godot_icall_int(___mb.mb_get_selection_from_line, (const Object *) this);
}

String TextEdit::get_selection_text() const {
	return ___godot_icall_String(___mb.mb_get_selection_text, (const Object *) this);
}

int64_t TextEdit::get_selection_to_column() const {
	return ___godot_icall_int(___mb.mb_get_selection_to_column, (const Object *) this);
}

int64_t TextEdit::get_selection_to_line() const {
	return ___godot_icall_int(___mb.mb_get_selection_to_line, (const Object *) this);
}

String TextEdit::get_text() {
	return ___godot_icall_String(___mb.mb_get_text, (const Object *) this);
}

real_t TextEdit::get_v_scroll() const {
	return ___godot_icall_float(___mb.mb_get_v_scroll, (const Object *) this);
}

real_t TextEdit::get_v_scroll_speed() const {
	return ___godot_icall_float(___mb.mb_get_v_scroll_speed, (const Object *) this);
}

String TextEdit::get_word_under_cursor() const {
	return ___godot_icall_String(___mb.mb_get_word_under_cursor, (const Object *) this);
}

bool TextEdit::has_keyword_color(const String keyword) const {
	return ___godot_icall_bool_String(___mb.mb_has_keyword_color, (const Object *) this, keyword);
}

void TextEdit::insert_text_at_cursor(const String text) {
	___godot_icall_void_String(___mb.mb_insert_text_at_cursor, (const Object *) this, text);
}

bool TextEdit::is_breakpoint_gutter_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_breakpoint_gutter_enabled, (const Object *) this);
}

bool TextEdit::is_context_menu_enabled() {
	return ___godot_icall_bool(___mb.mb_is_context_menu_enabled, (const Object *) this);
}

bool TextEdit::is_drawing_fold_gutter() const {
	return ___godot_icall_bool(___mb.mb_is_drawing_fold_gutter, (const Object *) this);
}

bool TextEdit::is_drawing_minimap() const {
	return ___godot_icall_bool(___mb.mb_is_drawing_minimap, (const Object *) this);
}

bool TextEdit::is_drawing_spaces() const {
	return ___godot_icall_bool(___mb.mb_is_drawing_spaces, (const Object *) this);
}

bool TextEdit::is_drawing_tabs() const {
	return ___godot_icall_bool(___mb.mb_is_drawing_tabs, (const Object *) this);
}

bool TextEdit::is_folded(const int64_t line) const {
	return ___godot_icall_bool_int(___mb.mb_is_folded, (const Object *) this, line);
}

bool TextEdit::is_hiding_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_hiding_enabled, (const Object *) this);
}

bool TextEdit::is_highlight_all_occurrences_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_highlight_all_occurrences_enabled, (const Object *) this);
}

bool TextEdit::is_highlight_current_line_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_highlight_current_line_enabled, (const Object *) this);
}

bool TextEdit::is_line_hidden(const int64_t line) const {
	return ___godot_icall_bool_int(___mb.mb_is_line_hidden, (const Object *) this, line);
}

bool TextEdit::is_overriding_selected_font_color() const {
	return ___godot_icall_bool(___mb.mb_is_overriding_selected_font_color, (const Object *) this);
}

bool TextEdit::is_readonly() const {
	return ___godot_icall_bool(___mb.mb_is_readonly, (const Object *) this);
}

bool TextEdit::is_right_click_moving_caret() const {
	return ___godot_icall_bool(___mb.mb_is_right_click_moving_caret, (const Object *) this);
}

bool TextEdit::is_selecting_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_selecting_enabled, (const Object *) this);
}

bool TextEdit::is_selection_active() const {
	return ___godot_icall_bool(___mb.mb_is_selection_active, (const Object *) this);
}

bool TextEdit::is_shortcut_keys_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_shortcut_keys_enabled, (const Object *) this);
}

bool TextEdit::is_show_line_numbers_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_show_line_numbers_enabled, (const Object *) this);
}

bool TextEdit::is_smooth_scroll_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_smooth_scroll_enabled, (const Object *) this);
}

bool TextEdit::is_syntax_coloring_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_syntax_coloring_enabled, (const Object *) this);
}

bool TextEdit::is_wrap_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_wrap_enabled, (const Object *) this);
}

void TextEdit::menu_option(const int64_t option) {
	___godot_icall_void_int(___mb.mb_menu_option, (const Object *) this, option);
}

void TextEdit::paste() {
	___godot_icall_void(___mb.mb_paste, (const Object *) this);
}

void TextEdit::redo() {
	___godot_icall_void(___mb.mb_redo, (const Object *) this);
}

void TextEdit::remove_breakpoints() {
	___godot_icall_void(___mb.mb_remove_breakpoints, (const Object *) this);
}

PoolIntArray TextEdit::search(const String key, const int64_t flags, const int64_t from_line, const int64_t from_column) const {
	return ___godot_icall_PoolIntArray_String_int_int_int(___mb.mb_search, (const Object *) this, key, flags, from_line, from_column);
}

void TextEdit::select(const int64_t from_line, const int64_t from_column, const int64_t to_line, const int64_t to_column) {
	___godot_icall_void_int_int_int_int(___mb.mb_select, (const Object *) this, from_line, from_column, to_line, to_column);
}

void TextEdit::select_all() {
	___godot_icall_void(___mb.mb_select_all, (const Object *) this);
}

void TextEdit::set_breakpoint_gutter_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_breakpoint_gutter_enabled, (const Object *) this, enable);
}

void TextEdit::set_context_menu_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_context_menu_enabled, (const Object *) this, enable);
}

void TextEdit::set_draw_fold_gutter(const bool arg0) {
	___godot_icall_void_bool(___mb.mb_set_draw_fold_gutter, (const Object *) this, arg0);
}

void TextEdit::set_draw_spaces(const bool arg0) {
	___godot_icall_void_bool(___mb.mb_set_draw_spaces, (const Object *) this, arg0);
}

void TextEdit::set_draw_tabs(const bool arg0) {
	___godot_icall_void_bool(___mb.mb_set_draw_tabs, (const Object *) this, arg0);
}

void TextEdit::set_h_scroll(const int64_t value) {
	___godot_icall_void_int(___mb.mb_set_h_scroll, (const Object *) this, value);
}

void TextEdit::set_hiding_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_hiding_enabled, (const Object *) this, enable);
}

void TextEdit::set_highlight_all_occurrences(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_highlight_all_occurrences, (const Object *) this, enable);
}

void TextEdit::set_highlight_current_line(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_highlight_current_line, (const Object *) this, enabled);
}

void TextEdit::set_line_as_hidden(const int64_t line, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_line_as_hidden, (const Object *) this, line, enable);
}

void TextEdit::set_minimap_width(const int64_t width) {
	___godot_icall_void_int(___mb.mb_set_minimap_width, (const Object *) this, width);
}

void TextEdit::set_override_selected_font_color(const bool override) {
	___godot_icall_void_bool(___mb.mb_set_override_selected_font_color, (const Object *) this, override);
}

void TextEdit::set_readonly(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_readonly, (const Object *) this, enable);
}

void TextEdit::set_right_click_moves_caret(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_right_click_moves_caret, (const Object *) this, enable);
}

void TextEdit::set_selecting_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_selecting_enabled, (const Object *) this, enable);
}

void TextEdit::set_shortcut_keys_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_shortcut_keys_enabled, (const Object *) this, enable);
}

void TextEdit::set_show_line_numbers(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_show_line_numbers, (const Object *) this, enable);
}

void TextEdit::set_smooth_scroll_enable(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_smooth_scroll_enable, (const Object *) this, enable);
}

void TextEdit::set_syntax_coloring(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_syntax_coloring, (const Object *) this, enable);
}

void TextEdit::set_text(const String text) {
	___godot_icall_void_String(___mb.mb_set_text, (const Object *) this, text);
}

void TextEdit::set_v_scroll(const real_t value) {
	___godot_icall_void_float(___mb.mb_set_v_scroll, (const Object *) this, value);
}

void TextEdit::set_v_scroll_speed(const real_t speed) {
	___godot_icall_void_float(___mb.mb_set_v_scroll_speed, (const Object *) this, speed);
}

void TextEdit::set_wrap_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_wrap_enabled, (const Object *) this, enable);
}

void TextEdit::toggle_fold_line(const int64_t line) {
	___godot_icall_void_int(___mb.mb_toggle_fold_line, (const Object *) this, line);
}

void TextEdit::undo() {
	___godot_icall_void(___mb.mb_undo, (const Object *) this);
}

void TextEdit::unfold_line(const int64_t line) {
	___godot_icall_void_int(___mb.mb_unfold_line, (const Object *) this, line);
}

void TextEdit::unhide_all_lines() {
	___godot_icall_void(___mb.mb_unhide_all_lines, (const Object *) this);
}

}