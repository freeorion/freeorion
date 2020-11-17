#include "OptionButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "PopupMenu.hpp"


namespace godot {


OptionButton::___method_bindings OptionButton::___mb = {};

void OptionButton::___init_method_bindings() {
	___mb.mb__focused = godot::api->godot_method_bind_get_method("OptionButton", "_focused");
	___mb.mb__get_items = godot::api->godot_method_bind_get_method("OptionButton", "_get_items");
	___mb.mb__select_int = godot::api->godot_method_bind_get_method("OptionButton", "_select_int");
	___mb.mb__selected = godot::api->godot_method_bind_get_method("OptionButton", "_selected");
	___mb.mb__set_items = godot::api->godot_method_bind_get_method("OptionButton", "_set_items");
	___mb.mb_add_icon_item = godot::api->godot_method_bind_get_method("OptionButton", "add_icon_item");
	___mb.mb_add_item = godot::api->godot_method_bind_get_method("OptionButton", "add_item");
	___mb.mb_add_separator = godot::api->godot_method_bind_get_method("OptionButton", "add_separator");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("OptionButton", "clear");
	___mb.mb_get_item_count = godot::api->godot_method_bind_get_method("OptionButton", "get_item_count");
	___mb.mb_get_item_icon = godot::api->godot_method_bind_get_method("OptionButton", "get_item_icon");
	___mb.mb_get_item_id = godot::api->godot_method_bind_get_method("OptionButton", "get_item_id");
	___mb.mb_get_item_index = godot::api->godot_method_bind_get_method("OptionButton", "get_item_index");
	___mb.mb_get_item_metadata = godot::api->godot_method_bind_get_method("OptionButton", "get_item_metadata");
	___mb.mb_get_item_text = godot::api->godot_method_bind_get_method("OptionButton", "get_item_text");
	___mb.mb_get_popup = godot::api->godot_method_bind_get_method("OptionButton", "get_popup");
	___mb.mb_get_selected = godot::api->godot_method_bind_get_method("OptionButton", "get_selected");
	___mb.mb_get_selected_id = godot::api->godot_method_bind_get_method("OptionButton", "get_selected_id");
	___mb.mb_get_selected_metadata = godot::api->godot_method_bind_get_method("OptionButton", "get_selected_metadata");
	___mb.mb_is_item_disabled = godot::api->godot_method_bind_get_method("OptionButton", "is_item_disabled");
	___mb.mb_remove_item = godot::api->godot_method_bind_get_method("OptionButton", "remove_item");
	___mb.mb_select = godot::api->godot_method_bind_get_method("OptionButton", "select");
	___mb.mb_set_item_disabled = godot::api->godot_method_bind_get_method("OptionButton", "set_item_disabled");
	___mb.mb_set_item_icon = godot::api->godot_method_bind_get_method("OptionButton", "set_item_icon");
	___mb.mb_set_item_id = godot::api->godot_method_bind_get_method("OptionButton", "set_item_id");
	___mb.mb_set_item_metadata = godot::api->godot_method_bind_get_method("OptionButton", "set_item_metadata");
	___mb.mb_set_item_text = godot::api->godot_method_bind_get_method("OptionButton", "set_item_text");
}

OptionButton *OptionButton::_new()
{
	return (OptionButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"OptionButton")());
}
void OptionButton::_focused(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__focused, (const Object *) this, arg0);
}

Array OptionButton::_get_items() const {
	return ___godot_icall_Array(___mb.mb__get_items, (const Object *) this);
}

void OptionButton::_select_int(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__select_int, (const Object *) this, arg0);
}

void OptionButton::_selected(const int64_t arg0) {
	___godot_icall_void_int(___mb.mb__selected, (const Object *) this, arg0);
}

void OptionButton::_set_items(const Array arg0) {
	___godot_icall_void_Array(___mb.mb__set_items, (const Object *) this, arg0);
}

void OptionButton::add_icon_item(const Ref<Texture> texture, const String label, const int64_t id) {
	___godot_icall_void_Object_String_int(___mb.mb_add_icon_item, (const Object *) this, texture.ptr(), label, id);
}

void OptionButton::add_item(const String label, const int64_t id) {
	___godot_icall_void_String_int(___mb.mb_add_item, (const Object *) this, label, id);
}

void OptionButton::add_separator() {
	___godot_icall_void(___mb.mb_add_separator, (const Object *) this);
}

void OptionButton::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

int64_t OptionButton::get_item_count() const {
	return ___godot_icall_int(___mb.mb_get_item_count, (const Object *) this);
}

Ref<Texture> OptionButton::get_item_icon(const int64_t idx) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_item_icon, (const Object *) this, idx));
}

int64_t OptionButton::get_item_id(const int64_t idx) const {
	return ___godot_icall_int_int(___mb.mb_get_item_id, (const Object *) this, idx);
}

int64_t OptionButton::get_item_index(const int64_t id) const {
	return ___godot_icall_int_int(___mb.mb_get_item_index, (const Object *) this, id);
}

Variant OptionButton::get_item_metadata(const int64_t idx) const {
	return ___godot_icall_Variant_int(___mb.mb_get_item_metadata, (const Object *) this, idx);
}

String OptionButton::get_item_text(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_item_text, (const Object *) this, idx);
}

PopupMenu *OptionButton::get_popup() const {
	return (PopupMenu *) ___godot_icall_Object(___mb.mb_get_popup, (const Object *) this);
}

int64_t OptionButton::get_selected() const {
	return ___godot_icall_int(___mb.mb_get_selected, (const Object *) this);
}

int64_t OptionButton::get_selected_id() const {
	return ___godot_icall_int(___mb.mb_get_selected_id, (const Object *) this);
}

Variant OptionButton::get_selected_metadata() const {
	return ___godot_icall_Variant(___mb.mb_get_selected_metadata, (const Object *) this);
}

bool OptionButton::is_item_disabled(const int64_t idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_item_disabled, (const Object *) this, idx);
}

void OptionButton::remove_item(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_remove_item, (const Object *) this, idx);
}

void OptionButton::select(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_select, (const Object *) this, idx);
}

void OptionButton::set_item_disabled(const int64_t idx, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_set_item_disabled, (const Object *) this, idx, disabled);
}

void OptionButton::set_item_icon(const int64_t idx, const Ref<Texture> texture) {
	___godot_icall_void_int_Object(___mb.mb_set_item_icon, (const Object *) this, idx, texture.ptr());
}

void OptionButton::set_item_id(const int64_t idx, const int64_t id) {
	___godot_icall_void_int_int(___mb.mb_set_item_id, (const Object *) this, idx, id);
}

void OptionButton::set_item_metadata(const int64_t idx, const Variant metadata) {
	___godot_icall_void_int_Variant(___mb.mb_set_item_metadata, (const Object *) this, idx, metadata);
}

void OptionButton::set_item_text(const int64_t idx, const String text) {
	___godot_icall_void_int_String(___mb.mb_set_item_text, (const Object *) this, idx, text);
}

}