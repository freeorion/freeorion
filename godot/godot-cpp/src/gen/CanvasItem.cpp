#include "CanvasItem.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Font.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include "MultiMesh.hpp"
#include "StyleBox.hpp"
#include "Material.hpp"
#include "World2D.hpp"
#include "InputEvent.hpp"


namespace godot {


CanvasItem::___method_bindings CanvasItem::___mb = {};

void CanvasItem::___init_method_bindings() {
	___mb.mb__draw = godot::api->godot_method_bind_get_method("CanvasItem", "_draw");
	___mb.mb__edit_get_pivot = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_get_pivot");
	___mb.mb__edit_get_position = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_get_position");
	___mb.mb__edit_get_rect = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_get_rect");
	___mb.mb__edit_get_rotation = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_get_rotation");
	___mb.mb__edit_get_scale = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_get_scale");
	___mb.mb__edit_get_state = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_get_state");
	___mb.mb__edit_get_transform = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_get_transform");
	___mb.mb__edit_set_pivot = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_set_pivot");
	___mb.mb__edit_set_position = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_set_position");
	___mb.mb__edit_set_rect = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_set_rect");
	___mb.mb__edit_set_rotation = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_set_rotation");
	___mb.mb__edit_set_scale = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_set_scale");
	___mb.mb__edit_set_state = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_set_state");
	___mb.mb__edit_use_pivot = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_use_pivot");
	___mb.mb__edit_use_rect = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_use_rect");
	___mb.mb__edit_use_rotation = godot::api->godot_method_bind_get_method("CanvasItem", "_edit_use_rotation");
	___mb.mb__is_on_top = godot::api->godot_method_bind_get_method("CanvasItem", "_is_on_top");
	___mb.mb__set_on_top = godot::api->godot_method_bind_get_method("CanvasItem", "_set_on_top");
	___mb.mb__toplevel_raise_self = godot::api->godot_method_bind_get_method("CanvasItem", "_toplevel_raise_self");
	___mb.mb__update_callback = godot::api->godot_method_bind_get_method("CanvasItem", "_update_callback");
	___mb.mb_draw_arc = godot::api->godot_method_bind_get_method("CanvasItem", "draw_arc");
	___mb.mb_draw_char = godot::api->godot_method_bind_get_method("CanvasItem", "draw_char");
	___mb.mb_draw_circle = godot::api->godot_method_bind_get_method("CanvasItem", "draw_circle");
	___mb.mb_draw_colored_polygon = godot::api->godot_method_bind_get_method("CanvasItem", "draw_colored_polygon");
	___mb.mb_draw_line = godot::api->godot_method_bind_get_method("CanvasItem", "draw_line");
	___mb.mb_draw_mesh = godot::api->godot_method_bind_get_method("CanvasItem", "draw_mesh");
	___mb.mb_draw_multiline = godot::api->godot_method_bind_get_method("CanvasItem", "draw_multiline");
	___mb.mb_draw_multiline_colors = godot::api->godot_method_bind_get_method("CanvasItem", "draw_multiline_colors");
	___mb.mb_draw_multimesh = godot::api->godot_method_bind_get_method("CanvasItem", "draw_multimesh");
	___mb.mb_draw_polygon = godot::api->godot_method_bind_get_method("CanvasItem", "draw_polygon");
	___mb.mb_draw_polyline = godot::api->godot_method_bind_get_method("CanvasItem", "draw_polyline");
	___mb.mb_draw_polyline_colors = godot::api->godot_method_bind_get_method("CanvasItem", "draw_polyline_colors");
	___mb.mb_draw_primitive = godot::api->godot_method_bind_get_method("CanvasItem", "draw_primitive");
	___mb.mb_draw_rect = godot::api->godot_method_bind_get_method("CanvasItem", "draw_rect");
	___mb.mb_draw_set_transform = godot::api->godot_method_bind_get_method("CanvasItem", "draw_set_transform");
	___mb.mb_draw_set_transform_matrix = godot::api->godot_method_bind_get_method("CanvasItem", "draw_set_transform_matrix");
	___mb.mb_draw_string = godot::api->godot_method_bind_get_method("CanvasItem", "draw_string");
	___mb.mb_draw_style_box = godot::api->godot_method_bind_get_method("CanvasItem", "draw_style_box");
	___mb.mb_draw_texture = godot::api->godot_method_bind_get_method("CanvasItem", "draw_texture");
	___mb.mb_draw_texture_rect = godot::api->godot_method_bind_get_method("CanvasItem", "draw_texture_rect");
	___mb.mb_draw_texture_rect_region = godot::api->godot_method_bind_get_method("CanvasItem", "draw_texture_rect_region");
	___mb.mb_force_update_transform = godot::api->godot_method_bind_get_method("CanvasItem", "force_update_transform");
	___mb.mb_get_canvas = godot::api->godot_method_bind_get_method("CanvasItem", "get_canvas");
	___mb.mb_get_canvas_item = godot::api->godot_method_bind_get_method("CanvasItem", "get_canvas_item");
	___mb.mb_get_canvas_transform = godot::api->godot_method_bind_get_method("CanvasItem", "get_canvas_transform");
	___mb.mb_get_global_mouse_position = godot::api->godot_method_bind_get_method("CanvasItem", "get_global_mouse_position");
	___mb.mb_get_global_transform = godot::api->godot_method_bind_get_method("CanvasItem", "get_global_transform");
	___mb.mb_get_global_transform_with_canvas = godot::api->godot_method_bind_get_method("CanvasItem", "get_global_transform_with_canvas");
	___mb.mb_get_light_mask = godot::api->godot_method_bind_get_method("CanvasItem", "get_light_mask");
	___mb.mb_get_local_mouse_position = godot::api->godot_method_bind_get_method("CanvasItem", "get_local_mouse_position");
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("CanvasItem", "get_material");
	___mb.mb_get_modulate = godot::api->godot_method_bind_get_method("CanvasItem", "get_modulate");
	___mb.mb_get_self_modulate = godot::api->godot_method_bind_get_method("CanvasItem", "get_self_modulate");
	___mb.mb_get_transform = godot::api->godot_method_bind_get_method("CanvasItem", "get_transform");
	___mb.mb_get_use_parent_material = godot::api->godot_method_bind_get_method("CanvasItem", "get_use_parent_material");
	___mb.mb_get_viewport_rect = godot::api->godot_method_bind_get_method("CanvasItem", "get_viewport_rect");
	___mb.mb_get_viewport_transform = godot::api->godot_method_bind_get_method("CanvasItem", "get_viewport_transform");
	___mb.mb_get_world_2d = godot::api->godot_method_bind_get_method("CanvasItem", "get_world_2d");
	___mb.mb_hide = godot::api->godot_method_bind_get_method("CanvasItem", "hide");
	___mb.mb_is_draw_behind_parent_enabled = godot::api->godot_method_bind_get_method("CanvasItem", "is_draw_behind_parent_enabled");
	___mb.mb_is_local_transform_notification_enabled = godot::api->godot_method_bind_get_method("CanvasItem", "is_local_transform_notification_enabled");
	___mb.mb_is_set_as_toplevel = godot::api->godot_method_bind_get_method("CanvasItem", "is_set_as_toplevel");
	___mb.mb_is_transform_notification_enabled = godot::api->godot_method_bind_get_method("CanvasItem", "is_transform_notification_enabled");
	___mb.mb_is_visible = godot::api->godot_method_bind_get_method("CanvasItem", "is_visible");
	___mb.mb_is_visible_in_tree = godot::api->godot_method_bind_get_method("CanvasItem", "is_visible_in_tree");
	___mb.mb_make_canvas_position_local = godot::api->godot_method_bind_get_method("CanvasItem", "make_canvas_position_local");
	___mb.mb_make_input_local = godot::api->godot_method_bind_get_method("CanvasItem", "make_input_local");
	___mb.mb_set_as_toplevel = godot::api->godot_method_bind_get_method("CanvasItem", "set_as_toplevel");
	___mb.mb_set_draw_behind_parent = godot::api->godot_method_bind_get_method("CanvasItem", "set_draw_behind_parent");
	___mb.mb_set_light_mask = godot::api->godot_method_bind_get_method("CanvasItem", "set_light_mask");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("CanvasItem", "set_material");
	___mb.mb_set_modulate = godot::api->godot_method_bind_get_method("CanvasItem", "set_modulate");
	___mb.mb_set_notify_local_transform = godot::api->godot_method_bind_get_method("CanvasItem", "set_notify_local_transform");
	___mb.mb_set_notify_transform = godot::api->godot_method_bind_get_method("CanvasItem", "set_notify_transform");
	___mb.mb_set_self_modulate = godot::api->godot_method_bind_get_method("CanvasItem", "set_self_modulate");
	___mb.mb_set_use_parent_material = godot::api->godot_method_bind_get_method("CanvasItem", "set_use_parent_material");
	___mb.mb_set_visible = godot::api->godot_method_bind_get_method("CanvasItem", "set_visible");
	___mb.mb_show = godot::api->godot_method_bind_get_method("CanvasItem", "show");
	___mb.mb_update = godot::api->godot_method_bind_get_method("CanvasItem", "update");
}

void CanvasItem::_draw() {
	___godot_icall_void(___mb.mb__draw, (const Object *) this);
}

Vector2 CanvasItem::_edit_get_pivot() const {
	return ___godot_icall_Vector2(___mb.mb__edit_get_pivot, (const Object *) this);
}

Vector2 CanvasItem::_edit_get_position() const {
	return ___godot_icall_Vector2(___mb.mb__edit_get_position, (const Object *) this);
}

Rect2 CanvasItem::_edit_get_rect() const {
	return ___godot_icall_Rect2(___mb.mb__edit_get_rect, (const Object *) this);
}

real_t CanvasItem::_edit_get_rotation() const {
	return ___godot_icall_float(___mb.mb__edit_get_rotation, (const Object *) this);
}

Vector2 CanvasItem::_edit_get_scale() const {
	return ___godot_icall_Vector2(___mb.mb__edit_get_scale, (const Object *) this);
}

Dictionary CanvasItem::_edit_get_state() const {
	return ___godot_icall_Dictionary(___mb.mb__edit_get_state, (const Object *) this);
}

Transform2D CanvasItem::_edit_get_transform() const {
	return ___godot_icall_Transform2D(___mb.mb__edit_get_transform, (const Object *) this);
}

void CanvasItem::_edit_set_pivot(const Vector2 pivot) {
	___godot_icall_void_Vector2(___mb.mb__edit_set_pivot, (const Object *) this, pivot);
}

void CanvasItem::_edit_set_position(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb__edit_set_position, (const Object *) this, position);
}

void CanvasItem::_edit_set_rect(const Rect2 rect) {
	___godot_icall_void_Rect2(___mb.mb__edit_set_rect, (const Object *) this, rect);
}

void CanvasItem::_edit_set_rotation(const real_t degrees) {
	___godot_icall_void_float(___mb.mb__edit_set_rotation, (const Object *) this, degrees);
}

void CanvasItem::_edit_set_scale(const Vector2 scale) {
	___godot_icall_void_Vector2(___mb.mb__edit_set_scale, (const Object *) this, scale);
}

void CanvasItem::_edit_set_state(const Dictionary state) {
	___godot_icall_void_Dictionary(___mb.mb__edit_set_state, (const Object *) this, state);
}

bool CanvasItem::_edit_use_pivot() const {
	return ___godot_icall_bool(___mb.mb__edit_use_pivot, (const Object *) this);
}

bool CanvasItem::_edit_use_rect() const {
	return ___godot_icall_bool(___mb.mb__edit_use_rect, (const Object *) this);
}

bool CanvasItem::_edit_use_rotation() const {
	return ___godot_icall_bool(___mb.mb__edit_use_rotation, (const Object *) this);
}

bool CanvasItem::_is_on_top() const {
	return ___godot_icall_bool(___mb.mb__is_on_top, (const Object *) this);
}

void CanvasItem::_set_on_top(const bool on_top) {
	___godot_icall_void_bool(___mb.mb__set_on_top, (const Object *) this, on_top);
}

void CanvasItem::_toplevel_raise_self() {
	___godot_icall_void(___mb.mb__toplevel_raise_self, (const Object *) this);
}

void CanvasItem::_update_callback() {
	___godot_icall_void(___mb.mb__update_callback, (const Object *) this);
}

void CanvasItem::draw_arc(const Vector2 center, const real_t radius, const real_t start_angle, const real_t end_angle, const int64_t point_count, const Color color, const real_t width, const bool antialiased) {
	___godot_icall_void_Vector2_float_float_float_int_Color_float_bool(___mb.mb_draw_arc, (const Object *) this, center, radius, start_angle, end_angle, point_count, color, width, antialiased);
}

real_t CanvasItem::draw_char(const Ref<Font> font, const Vector2 position, const String _char, const String next, const Color modulate) {
	return ___godot_icall_float_Object_Vector2_String_String_Color(___mb.mb_draw_char, (const Object *) this, font.ptr(), position, _char, next, modulate);
}

void CanvasItem::draw_circle(const Vector2 position, const real_t radius, const Color color) {
	___godot_icall_void_Vector2_float_Color(___mb.mb_draw_circle, (const Object *) this, position, radius, color);
}

void CanvasItem::draw_colored_polygon(const PoolVector2Array points, const Color color, const PoolVector2Array uvs, const Ref<Texture> texture, const Ref<Texture> normal_map, const bool antialiased) {
	___godot_icall_void_PoolVector2Array_Color_PoolVector2Array_Object_Object_bool(___mb.mb_draw_colored_polygon, (const Object *) this, points, color, uvs, texture.ptr(), normal_map.ptr(), antialiased);
}

void CanvasItem::draw_line(const Vector2 from, const Vector2 to, const Color color, const real_t width, const bool antialiased) {
	___godot_icall_void_Vector2_Vector2_Color_float_bool(___mb.mb_draw_line, (const Object *) this, from, to, color, width, antialiased);
}

void CanvasItem::draw_mesh(const Ref<Mesh> mesh, const Ref<Texture> texture, const Ref<Texture> normal_map, const Transform2D transform, const Color modulate) {
	___godot_icall_void_Object_Object_Object_Transform2D_Color(___mb.mb_draw_mesh, (const Object *) this, mesh.ptr(), texture.ptr(), normal_map.ptr(), transform, modulate);
}

void CanvasItem::draw_multiline(const PoolVector2Array points, const Color color, const real_t width, const bool antialiased) {
	___godot_icall_void_PoolVector2Array_Color_float_bool(___mb.mb_draw_multiline, (const Object *) this, points, color, width, antialiased);
}

void CanvasItem::draw_multiline_colors(const PoolVector2Array points, const PoolColorArray colors, const real_t width, const bool antialiased) {
	___godot_icall_void_PoolVector2Array_PoolColorArray_float_bool(___mb.mb_draw_multiline_colors, (const Object *) this, points, colors, width, antialiased);
}

void CanvasItem::draw_multimesh(const Ref<MultiMesh> multimesh, const Ref<Texture> texture, const Ref<Texture> normal_map) {
	___godot_icall_void_Object_Object_Object(___mb.mb_draw_multimesh, (const Object *) this, multimesh.ptr(), texture.ptr(), normal_map.ptr());
}

void CanvasItem::draw_polygon(const PoolVector2Array points, const PoolColorArray colors, const PoolVector2Array uvs, const Ref<Texture> texture, const Ref<Texture> normal_map, const bool antialiased) {
	___godot_icall_void_PoolVector2Array_PoolColorArray_PoolVector2Array_Object_Object_bool(___mb.mb_draw_polygon, (const Object *) this, points, colors, uvs, texture.ptr(), normal_map.ptr(), antialiased);
}

void CanvasItem::draw_polyline(const PoolVector2Array points, const Color color, const real_t width, const bool antialiased) {
	___godot_icall_void_PoolVector2Array_Color_float_bool(___mb.mb_draw_polyline, (const Object *) this, points, color, width, antialiased);
}

void CanvasItem::draw_polyline_colors(const PoolVector2Array points, const PoolColorArray colors, const real_t width, const bool antialiased) {
	___godot_icall_void_PoolVector2Array_PoolColorArray_float_bool(___mb.mb_draw_polyline_colors, (const Object *) this, points, colors, width, antialiased);
}

void CanvasItem::draw_primitive(const PoolVector2Array points, const PoolColorArray colors, const PoolVector2Array uvs, const Ref<Texture> texture, const real_t width, const Ref<Texture> normal_map) {
	___godot_icall_void_PoolVector2Array_PoolColorArray_PoolVector2Array_Object_float_Object(___mb.mb_draw_primitive, (const Object *) this, points, colors, uvs, texture.ptr(), width, normal_map.ptr());
}

void CanvasItem::draw_rect(const Rect2 rect, const Color color, const bool filled, const real_t width, const bool antialiased) {
	___godot_icall_void_Rect2_Color_bool_float_bool(___mb.mb_draw_rect, (const Object *) this, rect, color, filled, width, antialiased);
}

void CanvasItem::draw_set_transform(const Vector2 position, const real_t rotation, const Vector2 scale) {
	___godot_icall_void_Vector2_float_Vector2(___mb.mb_draw_set_transform, (const Object *) this, position, rotation, scale);
}

void CanvasItem::draw_set_transform_matrix(const Transform2D xform) {
	___godot_icall_void_Transform2D(___mb.mb_draw_set_transform_matrix, (const Object *) this, xform);
}

void CanvasItem::draw_string(const Ref<Font> font, const Vector2 position, const String text, const Color modulate, const int64_t clip_w) {
	___godot_icall_void_Object_Vector2_String_Color_int(___mb.mb_draw_string, (const Object *) this, font.ptr(), position, text, modulate, clip_w);
}

void CanvasItem::draw_style_box(const Ref<StyleBox> style_box, const Rect2 rect) {
	___godot_icall_void_Object_Rect2(___mb.mb_draw_style_box, (const Object *) this, style_box.ptr(), rect);
}

void CanvasItem::draw_texture(const Ref<Texture> texture, const Vector2 position, const Color modulate, const Ref<Texture> normal_map) {
	___godot_icall_void_Object_Vector2_Color_Object(___mb.mb_draw_texture, (const Object *) this, texture.ptr(), position, modulate, normal_map.ptr());
}

void CanvasItem::draw_texture_rect(const Ref<Texture> texture, const Rect2 rect, const bool tile, const Color modulate, const bool transpose, const Ref<Texture> normal_map) {
	___godot_icall_void_Object_Rect2_bool_Color_bool_Object(___mb.mb_draw_texture_rect, (const Object *) this, texture.ptr(), rect, tile, modulate, transpose, normal_map.ptr());
}

void CanvasItem::draw_texture_rect_region(const Ref<Texture> texture, const Rect2 rect, const Rect2 src_rect, const Color modulate, const bool transpose, const Ref<Texture> normal_map, const bool clip_uv) {
	___godot_icall_void_Object_Rect2_Rect2_Color_bool_Object_bool(___mb.mb_draw_texture_rect_region, (const Object *) this, texture.ptr(), rect, src_rect, modulate, transpose, normal_map.ptr(), clip_uv);
}

void CanvasItem::force_update_transform() {
	___godot_icall_void(___mb.mb_force_update_transform, (const Object *) this);
}

RID CanvasItem::get_canvas() const {
	return ___godot_icall_RID(___mb.mb_get_canvas, (const Object *) this);
}

RID CanvasItem::get_canvas_item() const {
	return ___godot_icall_RID(___mb.mb_get_canvas_item, (const Object *) this);
}

Transform2D CanvasItem::get_canvas_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_canvas_transform, (const Object *) this);
}

Vector2 CanvasItem::get_global_mouse_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_global_mouse_position, (const Object *) this);
}

Transform2D CanvasItem::get_global_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_global_transform, (const Object *) this);
}

Transform2D CanvasItem::get_global_transform_with_canvas() const {
	return ___godot_icall_Transform2D(___mb.mb_get_global_transform_with_canvas, (const Object *) this);
}

int64_t CanvasItem::get_light_mask() const {
	return ___godot_icall_int(___mb.mb_get_light_mask, (const Object *) this);
}

Vector2 CanvasItem::get_local_mouse_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_local_mouse_position, (const Object *) this);
}

Ref<Material> CanvasItem::get_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material, (const Object *) this));
}

Color CanvasItem::get_modulate() const {
	return ___godot_icall_Color(___mb.mb_get_modulate, (const Object *) this);
}

Color CanvasItem::get_self_modulate() const {
	return ___godot_icall_Color(___mb.mb_get_self_modulate, (const Object *) this);
}

Transform2D CanvasItem::get_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_transform, (const Object *) this);
}

bool CanvasItem::get_use_parent_material() const {
	return ___godot_icall_bool(___mb.mb_get_use_parent_material, (const Object *) this);
}

Rect2 CanvasItem::get_viewport_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_viewport_rect, (const Object *) this);
}

Transform2D CanvasItem::get_viewport_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_viewport_transform, (const Object *) this);
}

Ref<World2D> CanvasItem::get_world_2d() const {
	return Ref<World2D>::__internal_constructor(___godot_icall_Object(___mb.mb_get_world_2d, (const Object *) this));
}

void CanvasItem::hide() {
	___godot_icall_void(___mb.mb_hide, (const Object *) this);
}

bool CanvasItem::is_draw_behind_parent_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_draw_behind_parent_enabled, (const Object *) this);
}

bool CanvasItem::is_local_transform_notification_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_local_transform_notification_enabled, (const Object *) this);
}

bool CanvasItem::is_set_as_toplevel() const {
	return ___godot_icall_bool(___mb.mb_is_set_as_toplevel, (const Object *) this);
}

bool CanvasItem::is_transform_notification_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_transform_notification_enabled, (const Object *) this);
}

bool CanvasItem::is_visible() const {
	return ___godot_icall_bool(___mb.mb_is_visible, (const Object *) this);
}

bool CanvasItem::is_visible_in_tree() const {
	return ___godot_icall_bool(___mb.mb_is_visible_in_tree, (const Object *) this);
}

Vector2 CanvasItem::make_canvas_position_local(const Vector2 screen_point) const {
	return ___godot_icall_Vector2_Vector2(___mb.mb_make_canvas_position_local, (const Object *) this, screen_point);
}

Ref<InputEvent> CanvasItem::make_input_local(const Ref<InputEvent> event) const {
	return Ref<InputEvent>::__internal_constructor(___godot_icall_Object_Object(___mb.mb_make_input_local, (const Object *) this, event.ptr()));
}

void CanvasItem::set_as_toplevel(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_as_toplevel, (const Object *) this, enable);
}

void CanvasItem::set_draw_behind_parent(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_draw_behind_parent, (const Object *) this, enable);
}

void CanvasItem::set_light_mask(const int64_t light_mask) {
	___godot_icall_void_int(___mb.mb_set_light_mask, (const Object *) this, light_mask);
}

void CanvasItem::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

void CanvasItem::set_modulate(const Color modulate) {
	___godot_icall_void_Color(___mb.mb_set_modulate, (const Object *) this, modulate);
}

void CanvasItem::set_notify_local_transform(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_notify_local_transform, (const Object *) this, enable);
}

void CanvasItem::set_notify_transform(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_notify_transform, (const Object *) this, enable);
}

void CanvasItem::set_self_modulate(const Color self_modulate) {
	___godot_icall_void_Color(___mb.mb_set_self_modulate, (const Object *) this, self_modulate);
}

void CanvasItem::set_use_parent_material(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_parent_material, (const Object *) this, enable);
}

void CanvasItem::set_visible(const bool visible) {
	___godot_icall_void_bool(___mb.mb_set_visible, (const Object *) this, visible);
}

void CanvasItem::show() {
	___godot_icall_void(___mb.mb_show, (const Object *) this);
}

void CanvasItem::update() {
	___godot_icall_void(___mb.mb_update, (const Object *) this);
}

}