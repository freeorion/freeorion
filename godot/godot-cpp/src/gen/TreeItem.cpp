#include "TreeItem.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "TreeItem.hpp"
#include "Object.hpp"


namespace godot {


TreeItem::___method_bindings TreeItem::___mb = {};

void TreeItem::___init_method_bindings() {
	___mb.mb_add_button = godot::api->godot_method_bind_get_method("TreeItem", "add_button");
	___mb.mb_call_recursive = godot::api->godot_method_bind_get_method("TreeItem", "call_recursive");
	___mb.mb_clear_custom_bg_color = godot::api->godot_method_bind_get_method("TreeItem", "clear_custom_bg_color");
	___mb.mb_clear_custom_color = godot::api->godot_method_bind_get_method("TreeItem", "clear_custom_color");
	___mb.mb_deselect = godot::api->godot_method_bind_get_method("TreeItem", "deselect");
	___mb.mb_erase_button = godot::api->godot_method_bind_get_method("TreeItem", "erase_button");
	___mb.mb_get_button = godot::api->godot_method_bind_get_method("TreeItem", "get_button");
	___mb.mb_get_button_count = godot::api->godot_method_bind_get_method("TreeItem", "get_button_count");
	___mb.mb_get_button_tooltip = godot::api->godot_method_bind_get_method("TreeItem", "get_button_tooltip");
	___mb.mb_get_cell_mode = godot::api->godot_method_bind_get_method("TreeItem", "get_cell_mode");
	___mb.mb_get_children = godot::api->godot_method_bind_get_method("TreeItem", "get_children");
	___mb.mb_get_custom_bg_color = godot::api->godot_method_bind_get_method("TreeItem", "get_custom_bg_color");
	___mb.mb_get_custom_color = godot::api->godot_method_bind_get_method("TreeItem", "get_custom_color");
	___mb.mb_get_custom_minimum_height = godot::api->godot_method_bind_get_method("TreeItem", "get_custom_minimum_height");
	___mb.mb_get_expand_right = godot::api->godot_method_bind_get_method("TreeItem", "get_expand_right");
	___mb.mb_get_icon = godot::api->godot_method_bind_get_method("TreeItem", "get_icon");
	___mb.mb_get_icon_max_width = godot::api->godot_method_bind_get_method("TreeItem", "get_icon_max_width");
	___mb.mb_get_icon_modulate = godot::api->godot_method_bind_get_method("TreeItem", "get_icon_modulate");
	___mb.mb_get_icon_region = godot::api->godot_method_bind_get_method("TreeItem", "get_icon_region");
	___mb.mb_get_metadata = godot::api->godot_method_bind_get_method("TreeItem", "get_metadata");
	___mb.mb_get_next = godot::api->godot_method_bind_get_method("TreeItem", "get_next");
	___mb.mb_get_next_visible = godot::api->godot_method_bind_get_method("TreeItem", "get_next_visible");
	___mb.mb_get_parent = godot::api->godot_method_bind_get_method("TreeItem", "get_parent");
	___mb.mb_get_prev = godot::api->godot_method_bind_get_method("TreeItem", "get_prev");
	___mb.mb_get_prev_visible = godot::api->godot_method_bind_get_method("TreeItem", "get_prev_visible");
	___mb.mb_get_range = godot::api->godot_method_bind_get_method("TreeItem", "get_range");
	___mb.mb_get_range_config = godot::api->godot_method_bind_get_method("TreeItem", "get_range_config");
	___mb.mb_get_text = godot::api->godot_method_bind_get_method("TreeItem", "get_text");
	___mb.mb_get_text_align = godot::api->godot_method_bind_get_method("TreeItem", "get_text_align");
	___mb.mb_get_tooltip = godot::api->godot_method_bind_get_method("TreeItem", "get_tooltip");
	___mb.mb_is_button_disabled = godot::api->godot_method_bind_get_method("TreeItem", "is_button_disabled");
	___mb.mb_is_checked = godot::api->godot_method_bind_get_method("TreeItem", "is_checked");
	___mb.mb_is_collapsed = godot::api->godot_method_bind_get_method("TreeItem", "is_collapsed");
	___mb.mb_is_custom_set_as_button = godot::api->godot_method_bind_get_method("TreeItem", "is_custom_set_as_button");
	___mb.mb_is_editable = godot::api->godot_method_bind_get_method("TreeItem", "is_editable");
	___mb.mb_is_folding_disabled = godot::api->godot_method_bind_get_method("TreeItem", "is_folding_disabled");
	___mb.mb_is_selectable = godot::api->godot_method_bind_get_method("TreeItem", "is_selectable");
	___mb.mb_is_selected = godot::api->godot_method_bind_get_method("TreeItem", "is_selected");
	___mb.mb_move_to_bottom = godot::api->godot_method_bind_get_method("TreeItem", "move_to_bottom");
	___mb.mb_move_to_top = godot::api->godot_method_bind_get_method("TreeItem", "move_to_top");
	___mb.mb_remove_child = godot::api->godot_method_bind_get_method("TreeItem", "remove_child");
	___mb.mb_select = godot::api->godot_method_bind_get_method("TreeItem", "select");
	___mb.mb_set_button = godot::api->godot_method_bind_get_method("TreeItem", "set_button");
	___mb.mb_set_button_disabled = godot::api->godot_method_bind_get_method("TreeItem", "set_button_disabled");
	___mb.mb_set_cell_mode = godot::api->godot_method_bind_get_method("TreeItem", "set_cell_mode");
	___mb.mb_set_checked = godot::api->godot_method_bind_get_method("TreeItem", "set_checked");
	___mb.mb_set_collapsed = godot::api->godot_method_bind_get_method("TreeItem", "set_collapsed");
	___mb.mb_set_custom_as_button = godot::api->godot_method_bind_get_method("TreeItem", "set_custom_as_button");
	___mb.mb_set_custom_bg_color = godot::api->godot_method_bind_get_method("TreeItem", "set_custom_bg_color");
	___mb.mb_set_custom_color = godot::api->godot_method_bind_get_method("TreeItem", "set_custom_color");
	___mb.mb_set_custom_draw = godot::api->godot_method_bind_get_method("TreeItem", "set_custom_draw");
	___mb.mb_set_custom_minimum_height = godot::api->godot_method_bind_get_method("TreeItem", "set_custom_minimum_height");
	___mb.mb_set_disable_folding = godot::api->godot_method_bind_get_method("TreeItem", "set_disable_folding");
	___mb.mb_set_editable = godot::api->godot_method_bind_get_method("TreeItem", "set_editable");
	___mb.mb_set_expand_right = godot::api->godot_method_bind_get_method("TreeItem", "set_expand_right");
	___mb.mb_set_icon = godot::api->godot_method_bind_get_method("TreeItem", "set_icon");
	___mb.mb_set_icon_max_width = godot::api->godot_method_bind_get_method("TreeItem", "set_icon_max_width");
	___mb.mb_set_icon_modulate = godot::api->godot_method_bind_get_method("TreeItem", "set_icon_modulate");
	___mb.mb_set_icon_region = godot::api->godot_method_bind_get_method("TreeItem", "set_icon_region");
	___mb.mb_set_metadata = godot::api->godot_method_bind_get_method("TreeItem", "set_metadata");
	___mb.mb_set_range = godot::api->godot_method_bind_get_method("TreeItem", "set_range");
	___mb.mb_set_range_config = godot::api->godot_method_bind_get_method("TreeItem", "set_range_config");
	___mb.mb_set_selectable = godot::api->godot_method_bind_get_method("TreeItem", "set_selectable");
	___mb.mb_set_text = godot::api->godot_method_bind_get_method("TreeItem", "set_text");
	___mb.mb_set_text_align = godot::api->godot_method_bind_get_method("TreeItem", "set_text_align");
	___mb.mb_set_tooltip = godot::api->godot_method_bind_get_method("TreeItem", "set_tooltip");
}

void TreeItem::add_button(const int64_t column, const Ref<Texture> button, const int64_t button_idx, const bool disabled, const String tooltip) {
	___godot_icall_void_int_Object_int_bool_String(___mb.mb_add_button, (const Object *) this, column, button.ptr(), button_idx, disabled, tooltip);
}

Variant TreeItem::call_recursive(const String method, const Array& __var_args) {
	Variant __given_args[1];
	godot::api->godot_variant_new_nil((godot_variant *) &__given_args[0]);

	__given_args[0] = method;

	godot_variant **__args = (godot_variant **) alloca(sizeof(godot_variant *) * (__var_args.size() + 1));

	__args[0] = (godot_variant *) &__given_args[0];

	for (int i = 0; i < __var_args.size(); i++) {
		__args[i + 1] = (godot_variant *) &((Array &) __var_args)[i];
	}

	Variant __result;
	*(godot_variant *) &__result = godot::api->godot_method_bind_call(___mb.mb_call_recursive, ((const Object *) this)->_owner, (const godot_variant **) __args, (__var_args.size() + 1), nullptr);

	godot::api->godot_variant_destroy((godot_variant *) &__given_args[0]);

	return __result;
}

void TreeItem::clear_custom_bg_color(const int64_t column) {
	___godot_icall_void_int(___mb.mb_clear_custom_bg_color, (const Object *) this, column);
}

void TreeItem::clear_custom_color(const int64_t column) {
	___godot_icall_void_int(___mb.mb_clear_custom_color, (const Object *) this, column);
}

void TreeItem::deselect(const int64_t column) {
	___godot_icall_void_int(___mb.mb_deselect, (const Object *) this, column);
}

void TreeItem::erase_button(const int64_t column, const int64_t button_idx) {
	___godot_icall_void_int_int(___mb.mb_erase_button, (const Object *) this, column, button_idx);
}

Ref<Texture> TreeItem::get_button(const int64_t column, const int64_t button_idx) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int_int(___mb.mb_get_button, (const Object *) this, column, button_idx));
}

int64_t TreeItem::get_button_count(const int64_t column) const {
	return ___godot_icall_int_int(___mb.mb_get_button_count, (const Object *) this, column);
}

String TreeItem::get_button_tooltip(const int64_t column, const int64_t button_idx) const {
	return ___godot_icall_String_int_int(___mb.mb_get_button_tooltip, (const Object *) this, column, button_idx);
}

TreeItem::TreeCellMode TreeItem::get_cell_mode(const int64_t column) const {
	return (TreeItem::TreeCellMode) ___godot_icall_int_int(___mb.mb_get_cell_mode, (const Object *) this, column);
}

TreeItem *TreeItem::get_children() {
	return (TreeItem *) ___godot_icall_Object(___mb.mb_get_children, (const Object *) this);
}

Color TreeItem::get_custom_bg_color(const int64_t column) const {
	return ___godot_icall_Color_int(___mb.mb_get_custom_bg_color, (const Object *) this, column);
}

Color TreeItem::get_custom_color(const int64_t column) const {
	return ___godot_icall_Color_int(___mb.mb_get_custom_color, (const Object *) this, column);
}

int64_t TreeItem::get_custom_minimum_height() const {
	return ___godot_icall_int(___mb.mb_get_custom_minimum_height, (const Object *) this);
}

bool TreeItem::get_expand_right(const int64_t column) const {
	return ___godot_icall_bool_int(___mb.mb_get_expand_right, (const Object *) this, column);
}

Ref<Texture> TreeItem::get_icon(const int64_t column) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_icon, (const Object *) this, column));
}

int64_t TreeItem::get_icon_max_width(const int64_t column) const {
	return ___godot_icall_int_int(___mb.mb_get_icon_max_width, (const Object *) this, column);
}

Color TreeItem::get_icon_modulate(const int64_t column) const {
	return ___godot_icall_Color_int(___mb.mb_get_icon_modulate, (const Object *) this, column);
}

Rect2 TreeItem::get_icon_region(const int64_t column) const {
	return ___godot_icall_Rect2_int(___mb.mb_get_icon_region, (const Object *) this, column);
}

Variant TreeItem::get_metadata(const int64_t column) const {
	return ___godot_icall_Variant_int(___mb.mb_get_metadata, (const Object *) this, column);
}

TreeItem *TreeItem::get_next() {
	return (TreeItem *) ___godot_icall_Object(___mb.mb_get_next, (const Object *) this);
}

TreeItem *TreeItem::get_next_visible(const bool wrap) {
	return (TreeItem *) ___godot_icall_Object_bool(___mb.mb_get_next_visible, (const Object *) this, wrap);
}

TreeItem *TreeItem::get_parent() {
	return (TreeItem *) ___godot_icall_Object(___mb.mb_get_parent, (const Object *) this);
}

TreeItem *TreeItem::get_prev() {
	return (TreeItem *) ___godot_icall_Object(___mb.mb_get_prev, (const Object *) this);
}

TreeItem *TreeItem::get_prev_visible(const bool wrap) {
	return (TreeItem *) ___godot_icall_Object_bool(___mb.mb_get_prev_visible, (const Object *) this, wrap);
}

real_t TreeItem::get_range(const int64_t column) const {
	return ___godot_icall_float_int(___mb.mb_get_range, (const Object *) this, column);
}

Dictionary TreeItem::get_range_config(const int64_t column) {
	return ___godot_icall_Dictionary_int(___mb.mb_get_range_config, (const Object *) this, column);
}

String TreeItem::get_text(const int64_t column) const {
	return ___godot_icall_String_int(___mb.mb_get_text, (const Object *) this, column);
}

TreeItem::TextAlign TreeItem::get_text_align(const int64_t column) const {
	return (TreeItem::TextAlign) ___godot_icall_int_int(___mb.mb_get_text_align, (const Object *) this, column);
}

String TreeItem::get_tooltip(const int64_t column) const {
	return ___godot_icall_String_int(___mb.mb_get_tooltip, (const Object *) this, column);
}

bool TreeItem::is_button_disabled(const int64_t column, const int64_t button_idx) const {
	return ___godot_icall_bool_int_int(___mb.mb_is_button_disabled, (const Object *) this, column, button_idx);
}

bool TreeItem::is_checked(const int64_t column) const {
	return ___godot_icall_bool_int(___mb.mb_is_checked, (const Object *) this, column);
}

bool TreeItem::is_collapsed() {
	return ___godot_icall_bool(___mb.mb_is_collapsed, (const Object *) this);
}

bool TreeItem::is_custom_set_as_button(const int64_t column) const {
	return ___godot_icall_bool_int(___mb.mb_is_custom_set_as_button, (const Object *) this, column);
}

bool TreeItem::is_editable(const int64_t column) {
	return ___godot_icall_bool_int(___mb.mb_is_editable, (const Object *) this, column);
}

bool TreeItem::is_folding_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_folding_disabled, (const Object *) this);
}

bool TreeItem::is_selectable(const int64_t column) const {
	return ___godot_icall_bool_int(___mb.mb_is_selectable, (const Object *) this, column);
}

bool TreeItem::is_selected(const int64_t column) {
	return ___godot_icall_bool_int(___mb.mb_is_selected, (const Object *) this, column);
}

void TreeItem::move_to_bottom() {
	___godot_icall_void(___mb.mb_move_to_bottom, (const Object *) this);
}

void TreeItem::move_to_top() {
	___godot_icall_void(___mb.mb_move_to_top, (const Object *) this);
}

void TreeItem::remove_child(const Object *child) {
	___godot_icall_void_Object(___mb.mb_remove_child, (const Object *) this, child);
}

void TreeItem::select(const int64_t column) {
	___godot_icall_void_int(___mb.mb_select, (const Object *) this, column);
}

void TreeItem::set_button(const int64_t column, const int64_t button_idx, const Ref<Texture> button) {
	___godot_icall_void_int_int_Object(___mb.mb_set_button, (const Object *) this, column, button_idx, button.ptr());
}

void TreeItem::set_button_disabled(const int64_t column, const int64_t button_idx, const bool disabled) {
	___godot_icall_void_int_int_bool(___mb.mb_set_button_disabled, (const Object *) this, column, button_idx, disabled);
}

void TreeItem::set_cell_mode(const int64_t column, const int64_t mode) {
	___godot_icall_void_int_int(___mb.mb_set_cell_mode, (const Object *) this, column, mode);
}

void TreeItem::set_checked(const int64_t column, const bool checked) {
	___godot_icall_void_int_bool(___mb.mb_set_checked, (const Object *) this, column, checked);
}

void TreeItem::set_collapsed(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_collapsed, (const Object *) this, enable);
}

void TreeItem::set_custom_as_button(const int64_t column, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_custom_as_button, (const Object *) this, column, enable);
}

void TreeItem::set_custom_bg_color(const int64_t column, const Color color, const bool just_outline) {
	___godot_icall_void_int_Color_bool(___mb.mb_set_custom_bg_color, (const Object *) this, column, color, just_outline);
}

void TreeItem::set_custom_color(const int64_t column, const Color color) {
	___godot_icall_void_int_Color(___mb.mb_set_custom_color, (const Object *) this, column, color);
}

void TreeItem::set_custom_draw(const int64_t column, const Object *object, const String callback) {
	___godot_icall_void_int_Object_String(___mb.mb_set_custom_draw, (const Object *) this, column, object, callback);
}

void TreeItem::set_custom_minimum_height(const int64_t height) {
	___godot_icall_void_int(___mb.mb_set_custom_minimum_height, (const Object *) this, height);
}

void TreeItem::set_disable_folding(const bool disable) {
	___godot_icall_void_bool(___mb.mb_set_disable_folding, (const Object *) this, disable);
}

void TreeItem::set_editable(const int64_t column, const bool enabled) {
	___godot_icall_void_int_bool(___mb.mb_set_editable, (const Object *) this, column, enabled);
}

void TreeItem::set_expand_right(const int64_t column, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_expand_right, (const Object *) this, column, enable);
}

void TreeItem::set_icon(const int64_t column, const Ref<Texture> texture) {
	___godot_icall_void_int_Object(___mb.mb_set_icon, (const Object *) this, column, texture.ptr());
}

void TreeItem::set_icon_max_width(const int64_t column, const int64_t width) {
	___godot_icall_void_int_int(___mb.mb_set_icon_max_width, (const Object *) this, column, width);
}

void TreeItem::set_icon_modulate(const int64_t column, const Color modulate) {
	___godot_icall_void_int_Color(___mb.mb_set_icon_modulate, (const Object *) this, column, modulate);
}

void TreeItem::set_icon_region(const int64_t column, const Rect2 region) {
	___godot_icall_void_int_Rect2(___mb.mb_set_icon_region, (const Object *) this, column, region);
}

void TreeItem::set_metadata(const int64_t column, const Variant meta) {
	___godot_icall_void_int_Variant(___mb.mb_set_metadata, (const Object *) this, column, meta);
}

void TreeItem::set_range(const int64_t column, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_range, (const Object *) this, column, value);
}

void TreeItem::set_range_config(const int64_t column, const real_t min, const real_t max, const real_t step, const bool expr) {
	___godot_icall_void_int_float_float_float_bool(___mb.mb_set_range_config, (const Object *) this, column, min, max, step, expr);
}

void TreeItem::set_selectable(const int64_t column, const bool selectable) {
	___godot_icall_void_int_bool(___mb.mb_set_selectable, (const Object *) this, column, selectable);
}

void TreeItem::set_text(const int64_t column, const String text) {
	___godot_icall_void_int_String(___mb.mb_set_text, (const Object *) this, column, text);
}

void TreeItem::set_text_align(const int64_t column, const int64_t text_align) {
	___godot_icall_void_int_int(___mb.mb_set_text_align, (const Object *) this, column, text_align);
}

void TreeItem::set_tooltip(const int64_t column, const String tooltip) {
	___godot_icall_void_int_String(___mb.mb_set_tooltip, (const Object *) this, column, tooltip);
}

}