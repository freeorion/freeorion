#include "Control.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "Object.hpp"
#include "Font.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "StyleBox.hpp"
#include "Control.hpp"
#include "Theme.hpp"


namespace godot {


Control::___method_bindings Control::___mb = {};

void Control::___init_method_bindings() {
	___mb.mb__clips_input = godot::api->godot_method_bind_get_method("Control", "_clips_input");
	___mb.mb__get_minimum_size = godot::api->godot_method_bind_get_method("Control", "_get_minimum_size");
	___mb.mb__get_tooltip = godot::api->godot_method_bind_get_method("Control", "_get_tooltip");
	___mb.mb__gui_input = godot::api->godot_method_bind_get_method("Control", "_gui_input");
	___mb.mb__make_custom_tooltip = godot::api->godot_method_bind_get_method("Control", "_make_custom_tooltip");
	___mb.mb__override_changed = godot::api->godot_method_bind_get_method("Control", "_override_changed");
	___mb.mb__set_anchor = godot::api->godot_method_bind_get_method("Control", "_set_anchor");
	___mb.mb__set_global_position = godot::api->godot_method_bind_get_method("Control", "_set_global_position");
	___mb.mb__set_position = godot::api->godot_method_bind_get_method("Control", "_set_position");
	___mb.mb__set_size = godot::api->godot_method_bind_get_method("Control", "_set_size");
	___mb.mb__size_changed = godot::api->godot_method_bind_get_method("Control", "_size_changed");
	___mb.mb__theme_changed = godot::api->godot_method_bind_get_method("Control", "_theme_changed");
	___mb.mb__update_minimum_size = godot::api->godot_method_bind_get_method("Control", "_update_minimum_size");
	___mb.mb_accept_event = godot::api->godot_method_bind_get_method("Control", "accept_event");
	___mb.mb_add_color_override = godot::api->godot_method_bind_get_method("Control", "add_color_override");
	___mb.mb_add_constant_override = godot::api->godot_method_bind_get_method("Control", "add_constant_override");
	___mb.mb_add_font_override = godot::api->godot_method_bind_get_method("Control", "add_font_override");
	___mb.mb_add_icon_override = godot::api->godot_method_bind_get_method("Control", "add_icon_override");
	___mb.mb_add_shader_override = godot::api->godot_method_bind_get_method("Control", "add_shader_override");
	___mb.mb_add_stylebox_override = godot::api->godot_method_bind_get_method("Control", "add_stylebox_override");
	___mb.mb_can_drop_data = godot::api->godot_method_bind_get_method("Control", "can_drop_data");
	___mb.mb_drop_data = godot::api->godot_method_bind_get_method("Control", "drop_data");
	___mb.mb_force_drag = godot::api->godot_method_bind_get_method("Control", "force_drag");
	___mb.mb_get_anchor = godot::api->godot_method_bind_get_method("Control", "get_anchor");
	___mb.mb_get_begin = godot::api->godot_method_bind_get_method("Control", "get_begin");
	___mb.mb_get_color = godot::api->godot_method_bind_get_method("Control", "get_color");
	___mb.mb_get_combined_minimum_size = godot::api->godot_method_bind_get_method("Control", "get_combined_minimum_size");
	___mb.mb_get_constant = godot::api->godot_method_bind_get_method("Control", "get_constant");
	___mb.mb_get_cursor_shape = godot::api->godot_method_bind_get_method("Control", "get_cursor_shape");
	___mb.mb_get_custom_minimum_size = godot::api->godot_method_bind_get_method("Control", "get_custom_minimum_size");
	___mb.mb_get_default_cursor_shape = godot::api->godot_method_bind_get_method("Control", "get_default_cursor_shape");
	___mb.mb_get_drag_data = godot::api->godot_method_bind_get_method("Control", "get_drag_data");
	___mb.mb_get_end = godot::api->godot_method_bind_get_method("Control", "get_end");
	___mb.mb_get_focus_mode = godot::api->godot_method_bind_get_method("Control", "get_focus_mode");
	___mb.mb_get_focus_neighbour = godot::api->godot_method_bind_get_method("Control", "get_focus_neighbour");
	___mb.mb_get_focus_next = godot::api->godot_method_bind_get_method("Control", "get_focus_next");
	___mb.mb_get_focus_owner = godot::api->godot_method_bind_get_method("Control", "get_focus_owner");
	___mb.mb_get_focus_previous = godot::api->godot_method_bind_get_method("Control", "get_focus_previous");
	___mb.mb_get_font = godot::api->godot_method_bind_get_method("Control", "get_font");
	___mb.mb_get_global_position = godot::api->godot_method_bind_get_method("Control", "get_global_position");
	___mb.mb_get_global_rect = godot::api->godot_method_bind_get_method("Control", "get_global_rect");
	___mb.mb_get_h_grow_direction = godot::api->godot_method_bind_get_method("Control", "get_h_grow_direction");
	___mb.mb_get_h_size_flags = godot::api->godot_method_bind_get_method("Control", "get_h_size_flags");
	___mb.mb_get_icon = godot::api->godot_method_bind_get_method("Control", "get_icon");
	___mb.mb_get_margin = godot::api->godot_method_bind_get_method("Control", "get_margin");
	___mb.mb_get_minimum_size = godot::api->godot_method_bind_get_method("Control", "get_minimum_size");
	___mb.mb_get_mouse_filter = godot::api->godot_method_bind_get_method("Control", "get_mouse_filter");
	___mb.mb_get_parent_area_size = godot::api->godot_method_bind_get_method("Control", "get_parent_area_size");
	___mb.mb_get_parent_control = godot::api->godot_method_bind_get_method("Control", "get_parent_control");
	___mb.mb_get_pivot_offset = godot::api->godot_method_bind_get_method("Control", "get_pivot_offset");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("Control", "get_position");
	___mb.mb_get_rect = godot::api->godot_method_bind_get_method("Control", "get_rect");
	___mb.mb_get_rotation = godot::api->godot_method_bind_get_method("Control", "get_rotation");
	___mb.mb_get_rotation_degrees = godot::api->godot_method_bind_get_method("Control", "get_rotation_degrees");
	___mb.mb_get_scale = godot::api->godot_method_bind_get_method("Control", "get_scale");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("Control", "get_size");
	___mb.mb_get_stretch_ratio = godot::api->godot_method_bind_get_method("Control", "get_stretch_ratio");
	___mb.mb_get_stylebox = godot::api->godot_method_bind_get_method("Control", "get_stylebox");
	___mb.mb_get_theme = godot::api->godot_method_bind_get_method("Control", "get_theme");
	___mb.mb_get_tooltip = godot::api->godot_method_bind_get_method("Control", "get_tooltip");
	___mb.mb_get_v_grow_direction = godot::api->godot_method_bind_get_method("Control", "get_v_grow_direction");
	___mb.mb_get_v_size_flags = godot::api->godot_method_bind_get_method("Control", "get_v_size_flags");
	___mb.mb_grab_click_focus = godot::api->godot_method_bind_get_method("Control", "grab_click_focus");
	___mb.mb_grab_focus = godot::api->godot_method_bind_get_method("Control", "grab_focus");
	___mb.mb_has_color = godot::api->godot_method_bind_get_method("Control", "has_color");
	___mb.mb_has_color_override = godot::api->godot_method_bind_get_method("Control", "has_color_override");
	___mb.mb_has_constant = godot::api->godot_method_bind_get_method("Control", "has_constant");
	___mb.mb_has_constant_override = godot::api->godot_method_bind_get_method("Control", "has_constant_override");
	___mb.mb_has_focus = godot::api->godot_method_bind_get_method("Control", "has_focus");
	___mb.mb_has_font = godot::api->godot_method_bind_get_method("Control", "has_font");
	___mb.mb_has_font_override = godot::api->godot_method_bind_get_method("Control", "has_font_override");
	___mb.mb_has_icon = godot::api->godot_method_bind_get_method("Control", "has_icon");
	___mb.mb_has_icon_override = godot::api->godot_method_bind_get_method("Control", "has_icon_override");
	___mb.mb_has_point = godot::api->godot_method_bind_get_method("Control", "has_point");
	___mb.mb_has_shader_override = godot::api->godot_method_bind_get_method("Control", "has_shader_override");
	___mb.mb_has_stylebox = godot::api->godot_method_bind_get_method("Control", "has_stylebox");
	___mb.mb_has_stylebox_override = godot::api->godot_method_bind_get_method("Control", "has_stylebox_override");
	___mb.mb_is_clipping_contents = godot::api->godot_method_bind_get_method("Control", "is_clipping_contents");
	___mb.mb_minimum_size_changed = godot::api->godot_method_bind_get_method("Control", "minimum_size_changed");
	___mb.mb_release_focus = godot::api->godot_method_bind_get_method("Control", "release_focus");
	___mb.mb_set_anchor = godot::api->godot_method_bind_get_method("Control", "set_anchor");
	___mb.mb_set_anchor_and_margin = godot::api->godot_method_bind_get_method("Control", "set_anchor_and_margin");
	___mb.mb_set_anchors_and_margins_preset = godot::api->godot_method_bind_get_method("Control", "set_anchors_and_margins_preset");
	___mb.mb_set_anchors_preset = godot::api->godot_method_bind_get_method("Control", "set_anchors_preset");
	___mb.mb_set_begin = godot::api->godot_method_bind_get_method("Control", "set_begin");
	___mb.mb_set_clip_contents = godot::api->godot_method_bind_get_method("Control", "set_clip_contents");
	___mb.mb_set_custom_minimum_size = godot::api->godot_method_bind_get_method("Control", "set_custom_minimum_size");
	___mb.mb_set_default_cursor_shape = godot::api->godot_method_bind_get_method("Control", "set_default_cursor_shape");
	___mb.mb_set_drag_forwarding = godot::api->godot_method_bind_get_method("Control", "set_drag_forwarding");
	___mb.mb_set_drag_preview = godot::api->godot_method_bind_get_method("Control", "set_drag_preview");
	___mb.mb_set_end = godot::api->godot_method_bind_get_method("Control", "set_end");
	___mb.mb_set_focus_mode = godot::api->godot_method_bind_get_method("Control", "set_focus_mode");
	___mb.mb_set_focus_neighbour = godot::api->godot_method_bind_get_method("Control", "set_focus_neighbour");
	___mb.mb_set_focus_next = godot::api->godot_method_bind_get_method("Control", "set_focus_next");
	___mb.mb_set_focus_previous = godot::api->godot_method_bind_get_method("Control", "set_focus_previous");
	___mb.mb_set_global_position = godot::api->godot_method_bind_get_method("Control", "set_global_position");
	___mb.mb_set_h_grow_direction = godot::api->godot_method_bind_get_method("Control", "set_h_grow_direction");
	___mb.mb_set_h_size_flags = godot::api->godot_method_bind_get_method("Control", "set_h_size_flags");
	___mb.mb_set_margin = godot::api->godot_method_bind_get_method("Control", "set_margin");
	___mb.mb_set_margins_preset = godot::api->godot_method_bind_get_method("Control", "set_margins_preset");
	___mb.mb_set_mouse_filter = godot::api->godot_method_bind_get_method("Control", "set_mouse_filter");
	___mb.mb_set_pivot_offset = godot::api->godot_method_bind_get_method("Control", "set_pivot_offset");
	___mb.mb_set_position = godot::api->godot_method_bind_get_method("Control", "set_position");
	___mb.mb_set_rotation = godot::api->godot_method_bind_get_method("Control", "set_rotation");
	___mb.mb_set_rotation_degrees = godot::api->godot_method_bind_get_method("Control", "set_rotation_degrees");
	___mb.mb_set_scale = godot::api->godot_method_bind_get_method("Control", "set_scale");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("Control", "set_size");
	___mb.mb_set_stretch_ratio = godot::api->godot_method_bind_get_method("Control", "set_stretch_ratio");
	___mb.mb_set_theme = godot::api->godot_method_bind_get_method("Control", "set_theme");
	___mb.mb_set_tooltip = godot::api->godot_method_bind_get_method("Control", "set_tooltip");
	___mb.mb_set_v_grow_direction = godot::api->godot_method_bind_get_method("Control", "set_v_grow_direction");
	___mb.mb_set_v_size_flags = godot::api->godot_method_bind_get_method("Control", "set_v_size_flags");
	___mb.mb_show_modal = godot::api->godot_method_bind_get_method("Control", "show_modal");
	___mb.mb_warp_mouse = godot::api->godot_method_bind_get_method("Control", "warp_mouse");
}

Control *Control::_new()
{
	return (Control *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Control")());
}
bool Control::_clips_input() {
	return ___godot_icall_bool(___mb.mb__clips_input, (const Object *) this);
}

Vector2 Control::_get_minimum_size() {
	return ___godot_icall_Vector2(___mb.mb__get_minimum_size, (const Object *) this);
}

String Control::_get_tooltip() const {
	return ___godot_icall_String(___mb.mb__get_tooltip, (const Object *) this);
}

void Control::_gui_input(const Ref<InputEvent> event) {
	___godot_icall_void_Object(___mb.mb__gui_input, (const Object *) this, event.ptr());
}

Object *Control::_make_custom_tooltip(const String for_text) {
	return (Object *) ___godot_icall_Object_String(___mb.mb__make_custom_tooltip, (const Object *) this, for_text);
}

void Control::_override_changed() {
	___godot_icall_void(___mb.mb__override_changed, (const Object *) this);
}

void Control::_set_anchor(const int64_t margin, const real_t anchor) {
	___godot_icall_void_int_float(___mb.mb__set_anchor, (const Object *) this, margin, anchor);
}

void Control::_set_global_position(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb__set_global_position, (const Object *) this, position);
}

void Control::_set_position(const Vector2 margin) {
	___godot_icall_void_Vector2(___mb.mb__set_position, (const Object *) this, margin);
}

void Control::_set_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb__set_size, (const Object *) this, size);
}

void Control::_size_changed() {
	___godot_icall_void(___mb.mb__size_changed, (const Object *) this);
}

void Control::_theme_changed() {
	___godot_icall_void(___mb.mb__theme_changed, (const Object *) this);
}

void Control::_update_minimum_size() {
	___godot_icall_void(___mb.mb__update_minimum_size, (const Object *) this);
}

void Control::accept_event() {
	___godot_icall_void(___mb.mb_accept_event, (const Object *) this);
}

void Control::add_color_override(const String name, const Color color) {
	___godot_icall_void_String_Color(___mb.mb_add_color_override, (const Object *) this, name, color);
}

void Control::add_constant_override(const String name, const int64_t constant) {
	___godot_icall_void_String_int(___mb.mb_add_constant_override, (const Object *) this, name, constant);
}

void Control::add_font_override(const String name, const Ref<Font> font) {
	___godot_icall_void_String_Object(___mb.mb_add_font_override, (const Object *) this, name, font.ptr());
}

void Control::add_icon_override(const String name, const Ref<Texture> texture) {
	___godot_icall_void_String_Object(___mb.mb_add_icon_override, (const Object *) this, name, texture.ptr());
}

void Control::add_shader_override(const String name, const Ref<Shader> shader) {
	___godot_icall_void_String_Object(___mb.mb_add_shader_override, (const Object *) this, name, shader.ptr());
}

void Control::add_stylebox_override(const String name, const Ref<StyleBox> stylebox) {
	___godot_icall_void_String_Object(___mb.mb_add_stylebox_override, (const Object *) this, name, stylebox.ptr());
}

bool Control::can_drop_data(const Vector2 position, const Variant data) {
	return ___godot_icall_bool_Vector2_Variant(___mb.mb_can_drop_data, (const Object *) this, position, data);
}

void Control::drop_data(const Vector2 position, const Variant data) {
	___godot_icall_void_Vector2_Variant(___mb.mb_drop_data, (const Object *) this, position, data);
}

void Control::force_drag(const Variant data, const Control *preview) {
	___godot_icall_void_Variant_Object(___mb.mb_force_drag, (const Object *) this, data, preview);
}

real_t Control::get_anchor(const int64_t margin) const {
	return ___godot_icall_float_int(___mb.mb_get_anchor, (const Object *) this, margin);
}

Vector2 Control::get_begin() const {
	return ___godot_icall_Vector2(___mb.mb_get_begin, (const Object *) this);
}

Color Control::get_color(const String name, const String type) const {
	return ___godot_icall_Color_String_String(___mb.mb_get_color, (const Object *) this, name, type);
}

Vector2 Control::get_combined_minimum_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_combined_minimum_size, (const Object *) this);
}

int64_t Control::get_constant(const String name, const String type) const {
	return ___godot_icall_int_String_String(___mb.mb_get_constant, (const Object *) this, name, type);
}

Control::CursorShape Control::get_cursor_shape(const Vector2 position) const {
	return (Control::CursorShape) ___godot_icall_int_Vector2(___mb.mb_get_cursor_shape, (const Object *) this, position);
}

Vector2 Control::get_custom_minimum_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_custom_minimum_size, (const Object *) this);
}

Control::CursorShape Control::get_default_cursor_shape() const {
	return (Control::CursorShape) ___godot_icall_int(___mb.mb_get_default_cursor_shape, (const Object *) this);
}

Variant Control::get_drag_data(const Vector2 position) {
	return ___godot_icall_Variant_Vector2(___mb.mb_get_drag_data, (const Object *) this, position);
}

Vector2 Control::get_end() const {
	return ___godot_icall_Vector2(___mb.mb_get_end, (const Object *) this);
}

Control::FocusMode Control::get_focus_mode() const {
	return (Control::FocusMode) ___godot_icall_int(___mb.mb_get_focus_mode, (const Object *) this);
}

NodePath Control::get_focus_neighbour(const int64_t margin) const {
	return ___godot_icall_NodePath_int(___mb.mb_get_focus_neighbour, (const Object *) this, margin);
}

NodePath Control::get_focus_next() const {
	return ___godot_icall_NodePath(___mb.mb_get_focus_next, (const Object *) this);
}

Control *Control::get_focus_owner() const {
	return (Control *) ___godot_icall_Object(___mb.mb_get_focus_owner, (const Object *) this);
}

NodePath Control::get_focus_previous() const {
	return ___godot_icall_NodePath(___mb.mb_get_focus_previous, (const Object *) this);
}

Ref<Font> Control::get_font(const String name, const String type) const {
	return Ref<Font>::__internal_constructor(___godot_icall_Object_String_String(___mb.mb_get_font, (const Object *) this, name, type));
}

Vector2 Control::get_global_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_global_position, (const Object *) this);
}

Rect2 Control::get_global_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_global_rect, (const Object *) this);
}

Control::GrowDirection Control::get_h_grow_direction() const {
	return (Control::GrowDirection) ___godot_icall_int(___mb.mb_get_h_grow_direction, (const Object *) this);
}

int64_t Control::get_h_size_flags() const {
	return ___godot_icall_int(___mb.mb_get_h_size_flags, (const Object *) this);
}

Ref<Texture> Control::get_icon(const String name, const String type) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_String_String(___mb.mb_get_icon, (const Object *) this, name, type));
}

real_t Control::get_margin(const int64_t margin) const {
	return ___godot_icall_float_int(___mb.mb_get_margin, (const Object *) this, margin);
}

Vector2 Control::get_minimum_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_minimum_size, (const Object *) this);
}

Control::MouseFilter Control::get_mouse_filter() const {
	return (Control::MouseFilter) ___godot_icall_int(___mb.mb_get_mouse_filter, (const Object *) this);
}

Vector2 Control::get_parent_area_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_parent_area_size, (const Object *) this);
}

Control *Control::get_parent_control() const {
	return (Control *) ___godot_icall_Object(___mb.mb_get_parent_control, (const Object *) this);
}

Vector2 Control::get_pivot_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_pivot_offset, (const Object *) this);
}

Vector2 Control::get_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_position, (const Object *) this);
}

Rect2 Control::get_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_rect, (const Object *) this);
}

real_t Control::get_rotation() const {
	return ___godot_icall_float(___mb.mb_get_rotation, (const Object *) this);
}

real_t Control::get_rotation_degrees() const {
	return ___godot_icall_float(___mb.mb_get_rotation_degrees, (const Object *) this);
}

Vector2 Control::get_scale() const {
	return ___godot_icall_Vector2(___mb.mb_get_scale, (const Object *) this);
}

Vector2 Control::get_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_size, (const Object *) this);
}

real_t Control::get_stretch_ratio() const {
	return ___godot_icall_float(___mb.mb_get_stretch_ratio, (const Object *) this);
}

Ref<StyleBox> Control::get_stylebox(const String name, const String type) const {
	return Ref<StyleBox>::__internal_constructor(___godot_icall_Object_String_String(___mb.mb_get_stylebox, (const Object *) this, name, type));
}

Ref<Theme> Control::get_theme() const {
	return Ref<Theme>::__internal_constructor(___godot_icall_Object(___mb.mb_get_theme, (const Object *) this));
}

String Control::get_tooltip(const Vector2 at_position) const {
	return ___godot_icall_String_Vector2(___mb.mb_get_tooltip, (const Object *) this, at_position);
}

Control::GrowDirection Control::get_v_grow_direction() const {
	return (Control::GrowDirection) ___godot_icall_int(___mb.mb_get_v_grow_direction, (const Object *) this);
}

int64_t Control::get_v_size_flags() const {
	return ___godot_icall_int(___mb.mb_get_v_size_flags, (const Object *) this);
}

void Control::grab_click_focus() {
	___godot_icall_void(___mb.mb_grab_click_focus, (const Object *) this);
}

void Control::grab_focus() {
	___godot_icall_void(___mb.mb_grab_focus, (const Object *) this);
}

bool Control::has_color(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_color, (const Object *) this, name, type);
}

bool Control::has_color_override(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_color_override, (const Object *) this, name);
}

bool Control::has_constant(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_constant, (const Object *) this, name, type);
}

bool Control::has_constant_override(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_constant_override, (const Object *) this, name);
}

bool Control::has_focus() const {
	return ___godot_icall_bool(___mb.mb_has_focus, (const Object *) this);
}

bool Control::has_font(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_font, (const Object *) this, name, type);
}

bool Control::has_font_override(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_font_override, (const Object *) this, name);
}

bool Control::has_icon(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_icon, (const Object *) this, name, type);
}

bool Control::has_icon_override(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_icon_override, (const Object *) this, name);
}

bool Control::has_point(const Vector2 point) {
	return ___godot_icall_bool_Vector2(___mb.mb_has_point, (const Object *) this, point);
}

bool Control::has_shader_override(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_shader_override, (const Object *) this, name);
}

bool Control::has_stylebox(const String name, const String type) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_stylebox, (const Object *) this, name, type);
}

bool Control::has_stylebox_override(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_stylebox_override, (const Object *) this, name);
}

bool Control::is_clipping_contents() {
	return ___godot_icall_bool(___mb.mb_is_clipping_contents, (const Object *) this);
}

void Control::minimum_size_changed() {
	___godot_icall_void(___mb.mb_minimum_size_changed, (const Object *) this);
}

void Control::release_focus() {
	___godot_icall_void(___mb.mb_release_focus, (const Object *) this);
}

void Control::set_anchor(const int64_t margin, const real_t anchor, const bool keep_margin, const bool push_opposite_anchor) {
	___godot_icall_void_int_float_bool_bool(___mb.mb_set_anchor, (const Object *) this, margin, anchor, keep_margin, push_opposite_anchor);
}

void Control::set_anchor_and_margin(const int64_t margin, const real_t anchor, const real_t offset, const bool push_opposite_anchor) {
	___godot_icall_void_int_float_float_bool(___mb.mb_set_anchor_and_margin, (const Object *) this, margin, anchor, offset, push_opposite_anchor);
}

void Control::set_anchors_and_margins_preset(const int64_t preset, const int64_t resize_mode, const int64_t margin) {
	___godot_icall_void_int_int_int(___mb.mb_set_anchors_and_margins_preset, (const Object *) this, preset, resize_mode, margin);
}

void Control::set_anchors_preset(const int64_t preset, const bool keep_margins) {
	___godot_icall_void_int_bool(___mb.mb_set_anchors_preset, (const Object *) this, preset, keep_margins);
}

void Control::set_begin(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_set_begin, (const Object *) this, position);
}

void Control::set_clip_contents(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_clip_contents, (const Object *) this, enable);
}

void Control::set_custom_minimum_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_custom_minimum_size, (const Object *) this, size);
}

void Control::set_default_cursor_shape(const int64_t shape) {
	___godot_icall_void_int(___mb.mb_set_default_cursor_shape, (const Object *) this, shape);
}

void Control::set_drag_forwarding(const Control *target) {
	___godot_icall_void_Object(___mb.mb_set_drag_forwarding, (const Object *) this, target);
}

void Control::set_drag_preview(const Control *control) {
	___godot_icall_void_Object(___mb.mb_set_drag_preview, (const Object *) this, control);
}

void Control::set_end(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_set_end, (const Object *) this, position);
}

void Control::set_focus_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_focus_mode, (const Object *) this, mode);
}

void Control::set_focus_neighbour(const int64_t margin, const NodePath neighbour) {
	___godot_icall_void_int_NodePath(___mb.mb_set_focus_neighbour, (const Object *) this, margin, neighbour);
}

void Control::set_focus_next(const NodePath next) {
	___godot_icall_void_NodePath(___mb.mb_set_focus_next, (const Object *) this, next);
}

void Control::set_focus_previous(const NodePath previous) {
	___godot_icall_void_NodePath(___mb.mb_set_focus_previous, (const Object *) this, previous);
}

void Control::set_global_position(const Vector2 position, const bool keep_margins) {
	___godot_icall_void_Vector2_bool(___mb.mb_set_global_position, (const Object *) this, position, keep_margins);
}

void Control::set_h_grow_direction(const int64_t direction) {
	___godot_icall_void_int(___mb.mb_set_h_grow_direction, (const Object *) this, direction);
}

void Control::set_h_size_flags(const int64_t flags) {
	___godot_icall_void_int(___mb.mb_set_h_size_flags, (const Object *) this, flags);
}

void Control::set_margin(const int64_t margin, const real_t offset) {
	___godot_icall_void_int_float(___mb.mb_set_margin, (const Object *) this, margin, offset);
}

void Control::set_margins_preset(const int64_t preset, const int64_t resize_mode, const int64_t margin) {
	___godot_icall_void_int_int_int(___mb.mb_set_margins_preset, (const Object *) this, preset, resize_mode, margin);
}

void Control::set_mouse_filter(const int64_t filter) {
	___godot_icall_void_int(___mb.mb_set_mouse_filter, (const Object *) this, filter);
}

void Control::set_pivot_offset(const Vector2 pivot_offset) {
	___godot_icall_void_Vector2(___mb.mb_set_pivot_offset, (const Object *) this, pivot_offset);
}

void Control::set_position(const Vector2 position, const bool keep_margins) {
	___godot_icall_void_Vector2_bool(___mb.mb_set_position, (const Object *) this, position, keep_margins);
}

void Control::set_rotation(const real_t radians) {
	___godot_icall_void_float(___mb.mb_set_rotation, (const Object *) this, radians);
}

void Control::set_rotation_degrees(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_rotation_degrees, (const Object *) this, degrees);
}

void Control::set_scale(const Vector2 scale) {
	___godot_icall_void_Vector2(___mb.mb_set_scale, (const Object *) this, scale);
}

void Control::set_size(const Vector2 size, const bool keep_margins) {
	___godot_icall_void_Vector2_bool(___mb.mb_set_size, (const Object *) this, size, keep_margins);
}

void Control::set_stretch_ratio(const real_t ratio) {
	___godot_icall_void_float(___mb.mb_set_stretch_ratio, (const Object *) this, ratio);
}

void Control::set_theme(const Ref<Theme> theme) {
	___godot_icall_void_Object(___mb.mb_set_theme, (const Object *) this, theme.ptr());
}

void Control::set_tooltip(const String tooltip) {
	___godot_icall_void_String(___mb.mb_set_tooltip, (const Object *) this, tooltip);
}

void Control::set_v_grow_direction(const int64_t direction) {
	___godot_icall_void_int(___mb.mb_set_v_grow_direction, (const Object *) this, direction);
}

void Control::set_v_size_flags(const int64_t flags) {
	___godot_icall_void_int(___mb.mb_set_v_size_flags, (const Object *) this, flags);
}

void Control::show_modal(const bool exclusive) {
	___godot_icall_void_bool(___mb.mb_show_modal, (const Object *) this, exclusive);
}

void Control::warp_mouse(const Vector2 to_position) {
	___godot_icall_void_Vector2(___mb.mb_warp_mouse, (const Object *) this, to_position);
}

}