#include "Label.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Label::___method_bindings Label::___mb = {};

void Label::___init_method_bindings() {
	___mb.mb_get_align = godot::api->godot_method_bind_get_method("Label", "get_align");
	___mb.mb_get_line_count = godot::api->godot_method_bind_get_method("Label", "get_line_count");
	___mb.mb_get_line_height = godot::api->godot_method_bind_get_method("Label", "get_line_height");
	___mb.mb_get_lines_skipped = godot::api->godot_method_bind_get_method("Label", "get_lines_skipped");
	___mb.mb_get_max_lines_visible = godot::api->godot_method_bind_get_method("Label", "get_max_lines_visible");
	___mb.mb_get_percent_visible = godot::api->godot_method_bind_get_method("Label", "get_percent_visible");
	___mb.mb_get_text = godot::api->godot_method_bind_get_method("Label", "get_text");
	___mb.mb_get_total_character_count = godot::api->godot_method_bind_get_method("Label", "get_total_character_count");
	___mb.mb_get_valign = godot::api->godot_method_bind_get_method("Label", "get_valign");
	___mb.mb_get_visible_characters = godot::api->godot_method_bind_get_method("Label", "get_visible_characters");
	___mb.mb_get_visible_line_count = godot::api->godot_method_bind_get_method("Label", "get_visible_line_count");
	___mb.mb_has_autowrap = godot::api->godot_method_bind_get_method("Label", "has_autowrap");
	___mb.mb_is_clipping_text = godot::api->godot_method_bind_get_method("Label", "is_clipping_text");
	___mb.mb_is_uppercase = godot::api->godot_method_bind_get_method("Label", "is_uppercase");
	___mb.mb_set_align = godot::api->godot_method_bind_get_method("Label", "set_align");
	___mb.mb_set_autowrap = godot::api->godot_method_bind_get_method("Label", "set_autowrap");
	___mb.mb_set_clip_text = godot::api->godot_method_bind_get_method("Label", "set_clip_text");
	___mb.mb_set_lines_skipped = godot::api->godot_method_bind_get_method("Label", "set_lines_skipped");
	___mb.mb_set_max_lines_visible = godot::api->godot_method_bind_get_method("Label", "set_max_lines_visible");
	___mb.mb_set_percent_visible = godot::api->godot_method_bind_get_method("Label", "set_percent_visible");
	___mb.mb_set_text = godot::api->godot_method_bind_get_method("Label", "set_text");
	___mb.mb_set_uppercase = godot::api->godot_method_bind_get_method("Label", "set_uppercase");
	___mb.mb_set_valign = godot::api->godot_method_bind_get_method("Label", "set_valign");
	___mb.mb_set_visible_characters = godot::api->godot_method_bind_get_method("Label", "set_visible_characters");
}

Label *Label::_new()
{
	return (Label *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Label")());
}
Label::Align Label::get_align() const {
	return (Label::Align) ___godot_icall_int(___mb.mb_get_align, (const Object *) this);
}

int64_t Label::get_line_count() const {
	return ___godot_icall_int(___mb.mb_get_line_count, (const Object *) this);
}

int64_t Label::get_line_height() const {
	return ___godot_icall_int(___mb.mb_get_line_height, (const Object *) this);
}

int64_t Label::get_lines_skipped() const {
	return ___godot_icall_int(___mb.mb_get_lines_skipped, (const Object *) this);
}

int64_t Label::get_max_lines_visible() const {
	return ___godot_icall_int(___mb.mb_get_max_lines_visible, (const Object *) this);
}

real_t Label::get_percent_visible() const {
	return ___godot_icall_float(___mb.mb_get_percent_visible, (const Object *) this);
}

String Label::get_text() const {
	return ___godot_icall_String(___mb.mb_get_text, (const Object *) this);
}

int64_t Label::get_total_character_count() const {
	return ___godot_icall_int(___mb.mb_get_total_character_count, (const Object *) this);
}

Label::VAlign Label::get_valign() const {
	return (Label::VAlign) ___godot_icall_int(___mb.mb_get_valign, (const Object *) this);
}

int64_t Label::get_visible_characters() const {
	return ___godot_icall_int(___mb.mb_get_visible_characters, (const Object *) this);
}

int64_t Label::get_visible_line_count() const {
	return ___godot_icall_int(___mb.mb_get_visible_line_count, (const Object *) this);
}

bool Label::has_autowrap() const {
	return ___godot_icall_bool(___mb.mb_has_autowrap, (const Object *) this);
}

bool Label::is_clipping_text() const {
	return ___godot_icall_bool(___mb.mb_is_clipping_text, (const Object *) this);
}

bool Label::is_uppercase() const {
	return ___godot_icall_bool(___mb.mb_is_uppercase, (const Object *) this);
}

void Label::set_align(const int64_t align) {
	___godot_icall_void_int(___mb.mb_set_align, (const Object *) this, align);
}

void Label::set_autowrap(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_autowrap, (const Object *) this, enable);
}

void Label::set_clip_text(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_clip_text, (const Object *) this, enable);
}

void Label::set_lines_skipped(const int64_t lines_skipped) {
	___godot_icall_void_int(___mb.mb_set_lines_skipped, (const Object *) this, lines_skipped);
}

void Label::set_max_lines_visible(const int64_t lines_visible) {
	___godot_icall_void_int(___mb.mb_set_max_lines_visible, (const Object *) this, lines_visible);
}

void Label::set_percent_visible(const real_t percent_visible) {
	___godot_icall_void_float(___mb.mb_set_percent_visible, (const Object *) this, percent_visible);
}

void Label::set_text(const String text) {
	___godot_icall_void_String(___mb.mb_set_text, (const Object *) this, text);
}

void Label::set_uppercase(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_uppercase, (const Object *) this, enable);
}

void Label::set_valign(const int64_t valign) {
	___godot_icall_void_int(___mb.mb_set_valign, (const Object *) this, valign);
}

void Label::set_visible_characters(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_visible_characters, (const Object *) this, amount);
}

}