#include "LineEdit.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "PopupMenu.hpp"
#include "Texture.hpp"


namespace godot {


LineEdit::___method_bindings LineEdit::___mb = {};

void LineEdit::___init_method_bindings() {
	___mb.mb__editor_settings_changed = godot::api->godot_method_bind_get_method("LineEdit", "_editor_settings_changed");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("LineEdit", "_gui_input");
	___mb.mb__text_changed = godot::api->godot_method_bind_get_method("LineEdit", "_text_changed");
	___mb.mb__toggle_draw_caret = godot::api->godot_method_bind_get_method("LineEdit", "_toggle_draw_caret");
	___mb.mb_append_at_cursor = godot::api->godot_method_bind_get_method("LineEdit", "append_at_cursor");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("LineEdit", "clear");
	___mb.mb_cursor_get_blink_enabled = godot::api->godot_method_bind_get_method("LineEdit", "cursor_get_blink_enabled");
	___mb.mb_cursor_get_blink_speed = godot::api->godot_method_bind_get_method("LineEdit", "cursor_get_blink_speed");
	___mb.mb_cursor_set_blink_enabled = godot::api->godot_method_bind_get_method("LineEdit", "cursor_set_blink_enabled");
	___mb.mb_cursor_set_blink_speed = godot::api->godot_method_bind_get_method("LineEdit", "cursor_set_blink_speed");
	___mb.mb_deselect = godot::api->godot_method_bind_get_method("LineEdit", "deselect");
	___mb.mb_get_align = godot::api->godot_method_bind_get_method("LineEdit", "get_align");
	___mb.mb_get_cursor_position = godot::api->godot_method_bind_get_method("LineEdit", "get_cursor_position");
	___mb.mb_get_expand_to_text_length = godot::api->godot_method_bind_get_method("LineEdit", "get_expand_to_text_length");
	___mb.mb_get_max_length = godot::api->godot_method_bind_get_method("LineEdit", "get_max_length");
	___mb.mb_get_menu = godot::api->godot_method_bind_get_method("LineEdit", "get_menu");
	___mb.mb_get_placeholder = godot::api->godot_method_bind_get_method("LineEdit", "get_placeholder");
	___mb.mb_get_placeholder_alpha = godot::api->godot_method_bind_get_method("LineEdit", "get_placeholder_alpha");
	___mb.mb_get_right_icon = godot::api->godot_method_bind_get_method("LineEdit", "get_right_icon");
	___mb.mb_get_secret_character = godot::api->godot_method_bind_get_method("LineEdit", "get_secret_character");
	___mb.mb_get_text = godot::api->godot_method_bind_get_method("LineEdit", "get_text");
	___mb.mb_is_clear_button_enabled = godot::api->godot_method_bind_get_method("LineEdit", "is_clear_button_enabled");
	___mb.mb_is_context_menu_enabled = godot::api->godot_method_bind_get_method("LineEdit", "is_context_menu_enabled");
	___mb.mb_is_editable = godot::api->godot_method_bind_get_method("LineEdit", "is_editable");
	___mb.mb_is_secret = godot::api->godot_method_bind_get_method("LineEdit", "is_secret");
	___mb.mb_is_selecting_enabled = godot::api->godot_method_bind_get_method("LineEdit", "is_selecting_enabled");
	___mb.mb_is_shortcut_keys_enabled = godot::api->godot_method_bind_get_method("LineEdit", "is_shortcut_keys_enabled");
	___mb.mb_menu_option = godot::api->godot_method_bind_get_method("LineEdit", "menu_option");
	___mb.mb_select = godot::api->godot_method_bind_get_method("LineEdit", "select");
	___mb.mb_select_all = godot::api->godot_method_bind_get_method("LineEdit", "select_all");
	___mb.mb_set_align = godot::api->godot_method_bind_get_method("LineEdit", "set_align");
	___mb.mb_set_clear_button_enabled = godot::api->godot_method_bind_get_method("LineEdit", "set_clear_button_enabled");
	___mb.mb_set_context_menu_enabled = godot::api->godot_method_bind_get_method("LineEdit", "set_context_menu_enabled");
	___mb.mb_set_cursor_position = godot::api->godot_method_bind_get_method("LineEdit", "set_cursor_position");
	___mb.mb_set_editable = godot::api->godot_method_bind_get_method("LineEdit", "set_editable");
	___mb.mb_set_expand_to_text_length = godot::api->godot_method_bind_get_method("LineEdit", "set_expand_to_text_length");
	___mb.mb_set_max_length = godot::api->godot_method_bind_get_method("LineEdit", "set_max_length");
	___mb.mb_set_placeholder = godot::api->godot_method_bind_get_method("LineEdit", "set_placeholder");
	___mb.mb_set_placeholder_alpha = godot::api->godot_method_bind_get_method("LineEdit", "set_placeholder_alpha");
	___mb.mb_set_right_icon = godot::api->godot_method_bind_get_method("LineEdit", "set_right_icon");
	___mb.mb_set_secret = godot::api->godot_method_bind_get_method("LineEdit", "set_secret");
	___mb.mb_set_secret_character = godot::api->godot_method_bind_get_method("LineEdit", "set_secret_character");
	___mb.mb_set_selecting_enabled = godot::api->godot_method_bind_get_method("LineEdit", "set_selecting_enabled");
	___mb.mb_set_shortcut_keys_enabled = godot::api->godot_method_bind_get_method("LineEdit", "set_shortcut_keys_enabled");
	___mb.mb_set_text = godot::api->godot_method_bind_get_method("LineEdit", "set_text");
}

LineEdit *LineEdit::_new()
{
	return (LineEdit *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"LineEdit")());
}
void LineEdit::_editor_settings_changed() {
	___godot_icall_void(___mb.mb__editor_settings_changed, (const Object *) this);
}

void LineEdit::_gui_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, arg0.ptr());
}

void LineEdit::_text_changed() {
	___godot_icall_void(___mb.mb__text_changed, (const Object *) this);
}

void LineEdit::_toggle_draw_caret() {
	___godot_icall_void(___mb.mb__toggle_draw_caret, (const Object *) this);
}

void LineEdit::append_at_cursor(const String text) {
	___godot_icall_void_String(___mb.mb_append_at_cursor, (const Object *) this, text);
}

void LineEdit::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

bool LineEdit::cursor_get_blink_enabled() const {
	return ___godot_icall_bool(___mb.mb_cursor_get_blink_enabled, (const Object *) this);
}

real_t LineEdit::cursor_get_blink_speed() const {
	return ___godot_icall_float(___mb.mb_cursor_get_blink_speed, (const Object *) this);
}

void LineEdit::cursor_set_blink_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_cursor_set_blink_enabled, (const Object *) this, enabled);
}

void LineEdit::cursor_set_blink_speed(const real_t blink_speed) {
	___godot_icall_void_float(___mb.mb_cursor_set_blink_speed, (const Object *) this, blink_speed);
}

void LineEdit::deselect() {
	___godot_icall_void(___mb.mb_deselect, (const Object *) this);
}

LineEdit::Align LineEdit::get_align() const {
	return (LineEdit::Align) ___godot_icall_int(___mb.mb_get_align, (const Object *) this);
}

int64_t LineEdit::get_cursor_position() const {
	return ___godot_icall_int(___mb.mb_get_cursor_position, (const Object *) this);
}

bool LineEdit::get_expand_to_text_length() const {
	return ___godot_icall_bool(___mb.mb_get_expand_to_text_length, (const Object *) this);
}

int64_t LineEdit::get_max_length() const {
	return ___godot_icall_int(___mb.mb_get_max_length, (const Object *) this);
}

PopupMenu *LineEdit::get_menu() const {
	return (PopupMenu *) ___godot_icall_Object(___mb.mb_get_menu, (const Object *) this);
}

String LineEdit::get_placeholder() const {
	return ___godot_icall_String(___mb.mb_get_placeholder, (const Object *) this);
}

real_t LineEdit::get_placeholder_alpha() const {
	return ___godot_icall_float(___mb.mb_get_placeholder_alpha, (const Object *) this);
}

Ref<Texture> LineEdit::get_right_icon() {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_right_icon, (const Object *) this));
}

String LineEdit::get_secret_character() const {
	return ___godot_icall_String(___mb.mb_get_secret_character, (const Object *) this);
}

String LineEdit::get_text() const {
	return ___godot_icall_String(___mb.mb_get_text, (const Object *) this);
}

bool LineEdit::is_clear_button_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_clear_button_enabled, (const Object *) this);
}

bool LineEdit::is_context_menu_enabled() {
	return ___godot_icall_bool(___mb.mb_is_context_menu_enabled, (const Object *) this);
}

bool LineEdit::is_editable() const {
	return ___godot_icall_bool(___mb.mb_is_editable, (const Object *) this);
}

bool LineEdit::is_secret() const {
	return ___godot_icall_bool(___mb.mb_is_secret, (const Object *) this);
}

bool LineEdit::is_selecting_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_selecting_enabled, (const Object *) this);
}

bool LineEdit::is_shortcut_keys_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_shortcut_keys_enabled, (const Object *) this);
}

void LineEdit::menu_option(const int64_t option) {
	___godot_icall_void_int(___mb.mb_menu_option, (const Object *) this, option);
}

void LineEdit::select(const int64_t from, const int64_t to) {
	___godot_icall_void_int_int(___mb.mb_select, (const Object *) this, from, to);
}

void LineEdit::select_all() {
	___godot_icall_void(___mb.mb_select_all, (const Object *) this);
}

void LineEdit::set_align(const int64_t align) {
	___godot_icall_void_int(___mb.mb_set_align, (const Object *) this, align);
}

void LineEdit::set_clear_button_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_clear_button_enabled, (const Object *) this, enable);
}

void LineEdit::set_context_menu_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_context_menu_enabled, (const Object *) this, enable);
}

void LineEdit::set_cursor_position(const int64_t position) {
	___godot_icall_void_int(___mb.mb_set_cursor_position, (const Object *) this, position);
}

void LineEdit::set_editable(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_editable, (const Object *) this, enabled);
}

void LineEdit::set_expand_to_text_length(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_expand_to_text_length, (const Object *) this, enabled);
}

void LineEdit::set_max_length(const int64_t chars) {
	___godot_icall_void_int(___mb.mb_set_max_length, (const Object *) this, chars);
}

void LineEdit::set_placeholder(const String text) {
	___godot_icall_void_String(___mb.mb_set_placeholder, (const Object *) this, text);
}

void LineEdit::set_placeholder_alpha(const real_t alpha) {
	___godot_icall_void_float(___mb.mb_set_placeholder_alpha, (const Object *) this, alpha);
}

void LineEdit::set_right_icon(const Ref<Texture> icon) {
	___godot_icall_void_Object(___mb.mb_set_right_icon, (const Object *) this, icon.ptr());
}

void LineEdit::set_secret(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_secret, (const Object *) this, enabled);
}

void LineEdit::set_secret_character(const String character) {
	___godot_icall_void_String(___mb.mb_set_secret_character, (const Object *) this, character);
}

void LineEdit::set_selecting_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_selecting_enabled, (const Object *) this, enable);
}

void LineEdit::set_shortcut_keys_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_shortcut_keys_enabled, (const Object *) this, enable);
}

void LineEdit::set_text(const String text) {
	___godot_icall_void_String(___mb.mb_set_text, (const Object *) this, text);
}

}