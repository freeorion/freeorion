#include "RichTextLabel.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "Texture.hpp"
#include "VScrollBar.hpp"
#include "Font.hpp"


namespace godot {


RichTextLabel::___method_bindings RichTextLabel::___mb = {};

void RichTextLabel::___init_method_bindings() {
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("RichTextLabel", "_gui_input");
	___mb.mb__scroll_changed = godot::api->godot_method_bind_get_method("RichTextLabel", "_scroll_changed");
	___mb.mb_add_image = godot::api->godot_method_bind_get_method("RichTextLabel", "add_image");
	___mb.mb_add_text = godot::api->godot_method_bind_get_method("RichTextLabel", "add_text");
	___mb.mb_append_bbcode = godot::api->godot_method_bind_get_method("RichTextLabel", "append_bbcode");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("RichTextLabel", "clear");
	___mb.mb_get_bbcode = godot::api->godot_method_bind_get_method("RichTextLabel", "get_bbcode");
	___mb.mb_get_content_height = godot::api->godot_method_bind_get_method("RichTextLabel", "get_content_height");
	___mb.mb_get_effects = godot::api->godot_method_bind_get_method("RichTextLabel", "get_effects");
	___mb.mb_get_line_count = godot::api->godot_method_bind_get_method("RichTextLabel", "get_line_count");
	___mb.mb_get_percent_visible = godot::api->godot_method_bind_get_method("RichTextLabel", "get_percent_visible");
	___mb.mb_get_tab_size = godot::api->godot_method_bind_get_method("RichTextLabel", "get_tab_size");
	___mb.mb_get_text = godot::api->godot_method_bind_get_method("RichTextLabel", "get_text");
	___mb.mb_get_total_character_count = godot::api->godot_method_bind_get_method("RichTextLabel", "get_total_character_count");
	___mb.mb_get_v_scroll = godot::api->godot_method_bind_get_method("RichTextLabel", "get_v_scroll");
	___mb.mb_get_visible_characters = godot::api->godot_method_bind_get_method("RichTextLabel", "get_visible_characters");
	___mb.mb_get_visible_line_count = godot::api->godot_method_bind_get_method("RichTextLabel", "get_visible_line_count");
	___mb.mb_install_effect = godot::api->godot_method_bind_get_method("RichTextLabel", "install_effect");
	___mb.mb_is_meta_underlined = godot::api->godot_method_bind_get_method("RichTextLabel", "is_meta_underlined");
	___mb.mb_is_overriding_selected_font_color = godot::api->godot_method_bind_get_method("RichTextLabel", "is_overriding_selected_font_color");
	___mb.mb_is_scroll_active = godot::api->godot_method_bind_get_method("RichTextLabel", "is_scroll_active");
	___mb.mb_is_scroll_following = godot::api->godot_method_bind_get_method("RichTextLabel", "is_scroll_following");
	___mb.mb_is_selection_enabled = godot::api->godot_method_bind_get_method("RichTextLabel", "is_selection_enabled");
	___mb.mb_is_using_bbcode = godot::api->godot_method_bind_get_method("RichTextLabel", "is_using_bbcode");
	___mb.mb_newline = godot::api->godot_method_bind_get_method("RichTextLabel", "newline");
	___mb.mb_parse_bbcode = godot::api->godot_method_bind_get_method("RichTextLabel", "parse_bbcode");
	___mb.mb_parse_expressions_for_values = godot::api->godot_method_bind_get_method("RichTextLabel", "parse_expressions_for_values");
	___mb.mb_pop = godot::api->godot_method_bind_get_method("RichTextLabel", "pop");
	___mb.mb_push_align = godot::api->godot_method_bind_get_method("RichTextLabel", "push_align");
	___mb.mb_push_bold = godot::api->godot_method_bind_get_method("RichTextLabel", "push_bold");
	___mb.mb_push_bold_italics = godot::api->godot_method_bind_get_method("RichTextLabel", "push_bold_italics");
	___mb.mb_push_cell = godot::api->godot_method_bind_get_method("RichTextLabel", "push_cell");
	___mb.mb_push_color = godot::api->godot_method_bind_get_method("RichTextLabel", "push_color");
	___mb.mb_push_font = godot::api->godot_method_bind_get_method("RichTextLabel", "push_font");
	___mb.mb_push_indent = godot::api->godot_method_bind_get_method("RichTextLabel", "push_indent");
	___mb.mb_push_italics = godot::api->godot_method_bind_get_method("RichTextLabel", "push_italics");
	___mb.mb_push_list = godot::api->godot_method_bind_get_method("RichTextLabel", "push_list");
	___mb.mb_push_meta = godot::api->godot_method_bind_get_method("RichTextLabel", "push_meta");
	___mb.mb_push_mono = godot::api->godot_method_bind_get_method("RichTextLabel", "push_mono");
	___mb.mb_push_normal = godot::api->godot_method_bind_get_method("RichTextLabel", "push_normal");
	___mb.mb_push_strikethrough = godot::api->godot_method_bind_get_method("RichTextLabel", "push_strikethrough");
	___mb.mb_push_table = godot::api->godot_method_bind_get_method("RichTextLabel", "push_table");
	___mb.mb_push_underline = godot::api->godot_method_bind_get_method("RichTextLabel", "push_underline");
	___mb.mb_remove_line = godot::api->godot_method_bind_get_method("RichTextLabel", "remove_line");
	___mb.mb_scroll_to_line = godot::api->godot_method_bind_get_method("RichTextLabel", "scroll_to_line");
	___mb.mb_set_bbcode = godot::api->godot_method_bind_get_method("RichTextLabel", "set_bbcode");
	___mb.mb_set_effects = godot::api->godot_method_bind_get_method("RichTextLabel", "set_effects");
	___mb.mb_set_meta_underline = godot::api->godot_method_bind_get_method("RichTextLabel", "set_meta_underline");
	___mb.mb_set_override_selected_font_color = godot::api->godot_method_bind_get_method("RichTextLabel", "set_override_selected_font_color");
	___mb.mb_set_percent_visible = godot::api->godot_method_bind_get_method("RichTextLabel", "set_percent_visible");
	___mb.mb_set_scroll_active = godot::api->godot_method_bind_get_method("RichTextLabel", "set_scroll_active");
	___mb.mb_set_scroll_follow = godot::api->godot_method_bind_get_method("RichTextLabel", "set_scroll_follow");
	___mb.mb_set_selection_enabled = godot::api->godot_method_bind_get_method("RichTextLabel", "set_selection_enabled");
	___mb.mb_set_tab_size = godot::api->godot_method_bind_get_method("RichTextLabel", "set_tab_size");
	___mb.mb_set_table_column_expand = godot::api->godot_method_bind_get_method("RichTextLabel", "set_table_column_expand");
	___mb.mb_set_text = godot::api->godot_method_bind_get_method("RichTextLabel", "set_text");
	___mb.mb_set_use_bbcode = godot::api->godot_method_bind_get_method("RichTextLabel", "set_use_bbcode");
	___mb.mb_set_visible_characters = godot::api->godot_method_bind_get_method("RichTextLabel", "set_visible_characters");
}

RichTextLabel *RichTextLabel::_new()
{
	return (RichTextLabel *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"RichTextLabel")());
}
void RichTextLabel::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void RichTextLabel::_scroll_changed(const real_t arg0) {
	___godot_icall_void_float(___mb.mb__scroll_changed, (const Object *) this, arg0);
}

void RichTextLabel::add_image(const Ref<Texture> image, const int64_t width, const int64_t height) {
	___godot_icall_void_Object_int_int(___mb.mb_add_image, (const Object *) this, image.ptr(), width, height);
}

void RichTextLabel::add_text(const String text) {
	___godot_icall_void_String(___mb.mb_add_text, (const Object *) this, text);
}

Error RichTextLabel::append_bbcode(const String bbcode) {
	return (Error) ___godot_icall_int_String(___mb.mb_append_bbcode, (const Object *) this, bbcode);
}

void RichTextLabel::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

String RichTextLabel::get_bbcode() const {
	return ___godot_icall_String(___mb.mb_get_bbcode, (const Object *) this);
}

int64_t RichTextLabel::get_content_height() {
	return ___godot_icall_int(___mb.mb_get_content_height, (const Object *) this);
}

Array RichTextLabel::get_effects() {
	return ___godot_icall_Array(___mb.mb_get_effects, (const Object *) this);
}

int64_t RichTextLabel::get_line_count() const {
	return ___godot_icall_int(___mb.mb_get_line_count, (const Object *) this);
}

real_t RichTextLabel::get_percent_visible() const {
	return ___godot_icall_float(___mb.mb_get_percent_visible, (const Object *) this);
}

int64_t RichTextLabel::get_tab_size() const {
	return ___godot_icall_int(___mb.mb_get_tab_size, (const Object *) this);
}

String RichTextLabel::get_text() {
	return ___godot_icall_String(___mb.mb_get_text, (const Object *) this);
}

int64_t RichTextLabel::get_total_character_count() const {
	return ___godot_icall_int(___mb.mb_get_total_character_count, (const Object *) this);
}

VScrollBar *RichTextLabel::get_v_scroll() {
	return (VScrollBar *) ___godot_icall_Object(___mb.mb_get_v_scroll, (const Object *) this);
}

int64_t RichTextLabel::get_visible_characters() const {
	return ___godot_icall_int(___mb.mb_get_visible_characters, (const Object *) this);
}

int64_t RichTextLabel::get_visible_line_count() const {
	return ___godot_icall_int(___mb.mb_get_visible_line_count, (const Object *) this);
}

void RichTextLabel::install_effect(const Variant effect) {
	___godot_icall_void_Variant(___mb.mb_install_effect, (const Object *) this, effect);
}

bool RichTextLabel::is_meta_underlined() const {
	return ___godot_icall_bool(___mb.mb_is_meta_underlined, (const Object *) this);
}

bool RichTextLabel::is_overriding_selected_font_color() const {
	return ___godot_icall_bool(___mb.mb_is_overriding_selected_font_color, (const Object *) this);
}

bool RichTextLabel::is_scroll_active() const {
	return ___godot_icall_bool(___mb.mb_is_scroll_active, (const Object *) this);
}

bool RichTextLabel::is_scroll_following() const {
	return ___godot_icall_bool(___mb.mb_is_scroll_following, (const Object *) this);
}

bool RichTextLabel::is_selection_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_selection_enabled, (const Object *) this);
}

bool RichTextLabel::is_using_bbcode() const {
	return ___godot_icall_bool(___mb.mb_is_using_bbcode, (const Object *) this);
}

void RichTextLabel::newline() {
	___godot_icall_void(___mb.mb_newline, (const Object *) this);
}

Error RichTextLabel::parse_bbcode(const String bbcode) {
	return (Error) ___godot_icall_int_String(___mb.mb_parse_bbcode, (const Object *) this, bbcode);
}

Dictionary RichTextLabel::parse_expressions_for_values(const PoolStringArray expressions) {
	return ___godot_icall_Dictionary_PoolStringArray(___mb.mb_parse_expressions_for_values, (const Object *) this, expressions);
}

void RichTextLabel::pop() {
	___godot_icall_void(___mb.mb_pop, (const Object *) this);
}

void RichTextLabel::push_align(const int64_t align) {
	___godot_icall_void_int(___mb.mb_push_align, (const Object *) this, align);
}

void RichTextLabel::push_bold() {
	___godot_icall_void(___mb.mb_push_bold, (const Object *) this);
}

void RichTextLabel::push_bold_italics() {
	___godot_icall_void(___mb.mb_push_bold_italics, (const Object *) this);
}

void RichTextLabel::push_cell() {
	___godot_icall_void(___mb.mb_push_cell, (const Object *) this);
}

void RichTextLabel::push_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_push_color, (const Object *) this, color);
}

void RichTextLabel::push_font(const Ref<Font> font) {
	___godot_icall_void_Object(___mb.mb_push_font, (const Object *) this, font.ptr());
}

void RichTextLabel::push_indent(const int64_t level) {
	___godot_icall_void_int(___mb.mb_push_indent, (const Object *) this, level);
}

void RichTextLabel::push_italics() {
	___godot_icall_void(___mb.mb_push_italics, (const Object *) this);
}

void RichTextLabel::push_list(const int64_t type) {
	___godot_icall_void_int(___mb.mb_push_list, (const Object *) this, type);
}

void RichTextLabel::push_meta(const Variant data) {
	___godot_icall_void_Variant(___mb.mb_push_meta, (const Object *) this, data);
}

void RichTextLabel::push_mono() {
	___godot_icall_void(___mb.mb_push_mono, (const Object *) this);
}

void RichTextLabel::push_normal() {
	___godot_icall_void(___mb.mb_push_normal, (const Object *) this);
}

void RichTextLabel::push_strikethrough() {
	___godot_icall_void(___mb.mb_push_strikethrough, (const Object *) this);
}

void RichTextLabel::push_table(const int64_t columns) {
	___godot_icall_void_int(___mb.mb_push_table, (const Object *) this, columns);
}

void RichTextLabel::push_underline() {
	___godot_icall_void(___mb.mb_push_underline, (const Object *) this);
}

bool RichTextLabel::remove_line(const int64_t line) {
	return ___godot_icall_bool_int(___mb.mb_remove_line, (const Object *) this, line);
}

void RichTextLabel::scroll_to_line(const int64_t line) {
	___godot_icall_void_int(___mb.mb_scroll_to_line, (const Object *) this, line);
}

void RichTextLabel::set_bbcode(const String text) {
	___godot_icall_void_String(___mb.mb_set_bbcode, (const Object *) this, text);
}

void RichTextLabel::set_effects(const Array effects) {
	___godot_icall_void_Array(___mb.mb_set_effects, (const Object *) this, effects);
}

void RichTextLabel::set_meta_underline(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_meta_underline, (const Object *) this, enable);
}

void RichTextLabel::set_override_selected_font_color(const bool override) {
	___godot_icall_void_bool(___mb.mb_set_override_selected_font_color, (const Object *) this, override);
}

void RichTextLabel::set_percent_visible(const real_t percent_visible) {
	___godot_icall_void_float(___mb.mb_set_percent_visible, (const Object *) this, percent_visible);
}

void RichTextLabel::set_scroll_active(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_scroll_active, (const Object *) this, active);
}

void RichTextLabel::set_scroll_follow(const bool follow) {
	___godot_icall_void_bool(___mb.mb_set_scroll_follow, (const Object *) this, follow);
}

void RichTextLabel::set_selection_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_selection_enabled, (const Object *) this, enabled);
}

void RichTextLabel::set_tab_size(const int64_t spaces) {
	___godot_icall_void_int(___mb.mb_set_tab_size, (const Object *) this, spaces);
}

void RichTextLabel::set_table_column_expand(const int64_t column, const bool expand, const int64_t ratio) {
	___godot_icall_void_int_bool_int(___mb.mb_set_table_column_expand, (const Object *) this, column, expand, ratio);
}

void RichTextLabel::set_text(const String text) {
	___godot_icall_void_String(___mb.mb_set_text, (const Object *) this, text);
}

void RichTextLabel::set_use_bbcode(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_bbcode, (const Object *) this, enable);
}

void RichTextLabel::set_visible_characters(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_visible_characters, (const Object *) this, amount);
}

}