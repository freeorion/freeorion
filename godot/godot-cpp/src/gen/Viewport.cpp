#include "Viewport.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "World.hpp"
#include "World2D.hpp"
#include "Camera.hpp"
#include "Control.hpp"
#include "ViewportTexture.hpp"


namespace godot {


Viewport::___method_bindings Viewport::___mb = {};

void Viewport::___init_method_bindings() {
	___mb.mb__gui_remove_focus = godot::api->godot_method_bind_get_method("Viewport", "_gui_remove_focus");
	___mb.mb__gui_show_tooltip = godot::api->godot_method_bind_get_method("Viewport", "_gui_show_tooltip");
	___mb.mb__own_world_changed = godot::api->godot_method_bind_get_method("Viewport", "_own_world_changed");
	___mb.mb__post_gui_grab_click_focus = godot::api->godot_method_bind_get_method("Viewport", "_post_gui_grab_click_focus");
	___mb.mb__subwindow_visibility_changed = godot::api->godot_method_bind_get_method("Viewport", "_subwindow_visibility_changed");
	___mb.mb__vp_input = godot::api->godot_method_bind_get_method("Viewport", "_vp_input");
	___mb.mb__vp_input_text = godot::api->godot_method_bind_get_method("Viewport", "_vp_input_text");
	___mb.mb__vp_unhandled_input = godot::api->godot_method_bind_get_method("Viewport", "_vp_unhandled_input");
	___mb.mb_find_world = godot::api->godot_method_bind_get_method("Viewport", "find_world");
	___mb.mb_find_world_2d = godot::api->godot_method_bind_get_method("Viewport", "find_world_2d");
	___mb.mb_get_camera = godot::api->godot_method_bind_get_method("Viewport", "get_camera");
	___mb.mb_get_canvas_transform = godot::api->godot_method_bind_get_method("Viewport", "get_canvas_transform");
	___mb.mb_get_clear_mode = godot::api->godot_method_bind_get_method("Viewport", "get_clear_mode");
	___mb.mb_get_debug_draw = godot::api->godot_method_bind_get_method("Viewport", "get_debug_draw");
	___mb.mb_get_final_transform = godot::api->godot_method_bind_get_method("Viewport", "get_final_transform");
	___mb.mb_get_global_canvas_transform = godot::api->godot_method_bind_get_method("Viewport", "get_global_canvas_transform");
	___mb.mb_get_hdr = godot::api->godot_method_bind_get_method("Viewport", "get_hdr");
	___mb.mb_get_keep_3d_linear = godot::api->godot_method_bind_get_method("Viewport", "get_keep_3d_linear");
	___mb.mb_get_modal_stack_top = godot::api->godot_method_bind_get_method("Viewport", "get_modal_stack_top");
	___mb.mb_get_mouse_position = godot::api->godot_method_bind_get_method("Viewport", "get_mouse_position");
	___mb.mb_get_msaa = godot::api->godot_method_bind_get_method("Viewport", "get_msaa");
	___mb.mb_get_physics_object_picking = godot::api->godot_method_bind_get_method("Viewport", "get_physics_object_picking");
	___mb.mb_get_render_info = godot::api->godot_method_bind_get_method("Viewport", "get_render_info");
	___mb.mb_get_shadow_atlas_quadrant_subdiv = godot::api->godot_method_bind_get_method("Viewport", "get_shadow_atlas_quadrant_subdiv");
	___mb.mb_get_shadow_atlas_size = godot::api->godot_method_bind_get_method("Viewport", "get_shadow_atlas_size");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("Viewport", "get_size");
	___mb.mb_get_size_override = godot::api->godot_method_bind_get_method("Viewport", "get_size_override");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("Viewport", "get_texture");
	___mb.mb_get_update_mode = godot::api->godot_method_bind_get_method("Viewport", "get_update_mode");
	___mb.mb_get_usage = godot::api->godot_method_bind_get_method("Viewport", "get_usage");
	___mb.mb_get_vflip = godot::api->godot_method_bind_get_method("Viewport", "get_vflip");
	___mb.mb_get_viewport_rid = godot::api->godot_method_bind_get_method("Viewport", "get_viewport_rid");
	___mb.mb_get_visible_rect = godot::api->godot_method_bind_get_method("Viewport", "get_visible_rect");
	___mb.mb_get_world = godot::api->godot_method_bind_get_method("Viewport", "get_world");
	___mb.mb_get_world_2d = godot::api->godot_method_bind_get_method("Viewport", "get_world_2d");
	___mb.mb_gui_get_drag_data = godot::api->godot_method_bind_get_method("Viewport", "gui_get_drag_data");
	___mb.mb_gui_has_modal_stack = godot::api->godot_method_bind_get_method("Viewport", "gui_has_modal_stack");
	___mb.mb_gui_is_dragging = godot::api->godot_method_bind_get_method("Viewport", "gui_is_dragging");
	___mb.mb_has_transparent_background = godot::api->godot_method_bind_get_method("Viewport", "has_transparent_background");
	___mb.mb_input = godot::api->godot_method_bind_get_method("Viewport", "input");
	___mb.mb_is_3d_disabled = godot::api->godot_method_bind_get_method("Viewport", "is_3d_disabled");
	___mb.mb_is_audio_listener = godot::api->godot_method_bind_get_method("Viewport", "is_audio_listener");
	___mb.mb_is_audio_listener_2d = godot::api->godot_method_bind_get_method("Viewport", "is_audio_listener_2d");
	___mb.mb_is_handling_input_locally = godot::api->godot_method_bind_get_method("Viewport", "is_handling_input_locally");
	___mb.mb_is_input_disabled = godot::api->godot_method_bind_get_method("Viewport", "is_input_disabled");
	___mb.mb_is_input_handled = godot::api->godot_method_bind_get_method("Viewport", "is_input_handled");
	___mb.mb_is_size_override_enabled = godot::api->godot_method_bind_get_method("Viewport", "is_size_override_enabled");
	___mb.mb_is_size_override_stretch_enabled = godot::api->godot_method_bind_get_method("Viewport", "is_size_override_stretch_enabled");
	___mb.mb_is_snap_controls_to_pixels_enabled = godot::api->godot_method_bind_get_method("Viewport", "is_snap_controls_to_pixels_enabled");
	___mb.mb_is_using_own_world = godot::api->godot_method_bind_get_method("Viewport", "is_using_own_world");
	___mb.mb_is_using_render_direct_to_screen = godot::api->godot_method_bind_get_method("Viewport", "is_using_render_direct_to_screen");
	___mb.mb_set_as_audio_listener = godot::api->godot_method_bind_get_method("Viewport", "set_as_audio_listener");
	___mb.mb_set_as_audio_listener_2d = godot::api->godot_method_bind_get_method("Viewport", "set_as_audio_listener_2d");
	___mb.mb_set_attach_to_screen_rect = godot::api->godot_method_bind_get_method("Viewport", "set_attach_to_screen_rect");
	___mb.mb_set_canvas_transform = godot::api->godot_method_bind_get_method("Viewport", "set_canvas_transform");
	___mb.mb_set_clear_mode = godot::api->godot_method_bind_get_method("Viewport", "set_clear_mode");
	___mb.mb_set_debug_draw = godot::api->godot_method_bind_get_method("Viewport", "set_debug_draw");
	___mb.mb_set_disable_3d = godot::api->godot_method_bind_get_method("Viewport", "set_disable_3d");
	___mb.mb_set_disable_input = godot::api->godot_method_bind_get_method("Viewport", "set_disable_input");
	___mb.mb_set_global_canvas_transform = godot::api->godot_method_bind_get_method("Viewport", "set_global_canvas_transform");
	___mb.mb_set_handle_input_locally = godot::api->godot_method_bind_get_method("Viewport", "set_handle_input_locally");
	___mb.mb_set_hdr = godot::api->godot_method_bind_get_method("Viewport", "set_hdr");
	___mb.mb_set_input_as_handled = godot::api->godot_method_bind_get_method("Viewport", "set_input_as_handled");
	___mb.mb_set_keep_3d_linear = godot::api->godot_method_bind_get_method("Viewport", "set_keep_3d_linear");
	___mb.mb_set_msaa = godot::api->godot_method_bind_get_method("Viewport", "set_msaa");
	___mb.mb_set_physics_object_picking = godot::api->godot_method_bind_get_method("Viewport", "set_physics_object_picking");
	___mb.mb_set_shadow_atlas_quadrant_subdiv = godot::api->godot_method_bind_get_method("Viewport", "set_shadow_atlas_quadrant_subdiv");
	___mb.mb_set_shadow_atlas_size = godot::api->godot_method_bind_get_method("Viewport", "set_shadow_atlas_size");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("Viewport", "set_size");
	___mb.mb_set_size_override = godot::api->godot_method_bind_get_method("Viewport", "set_size_override");
	___mb.mb_set_size_override_stretch = godot::api->godot_method_bind_get_method("Viewport", "set_size_override_stretch");
	___mb.mb_set_snap_controls_to_pixels = godot::api->godot_method_bind_get_method("Viewport", "set_snap_controls_to_pixels");
	___mb.mb_set_transparent_background = godot::api->godot_method_bind_get_method("Viewport", "set_transparent_background");
	___mb.mb_set_update_mode = godot::api->godot_method_bind_get_method("Viewport", "set_update_mode");
	___mb.mb_set_usage = godot::api->godot_method_bind_get_method("Viewport", "set_usage");
	___mb.mb_set_use_arvr = godot::api->godot_method_bind_get_method("Viewport", "set_use_arvr");
	___mb.mb_set_use_own_world = godot::api->godot_method_bind_get_method("Viewport", "set_use_own_world");
	___mb.mb_set_use_render_direct_to_screen = godot::api->godot_method_bind_get_method("Viewport", "set_use_render_direct_to_screen");
	___mb.mb_set_vflip = godot::api->godot_method_bind_get_method("Viewport", "set_vflip");
	___mb.mb_set_world = godot::api->godot_method_bind_get_method("Viewport", "set_world");
	___mb.mb_set_world_2d = godot::api->godot_method_bind_get_method("Viewport", "set_world_2d");
	___mb.mb_unhandled_input = godot::api->godot_method_bind_get_method("Viewport", "unhandled_input");
	___mb.mb_update_worlds = godot::api->godot_method_bind_get_method("Viewport", "update_worlds");
	___mb.mb_use_arvr = godot::api->godot_method_bind_get_method("Viewport", "use_arvr");
	___mb.mb_warp_mouse = godot::api->godot_method_bind_get_method("Viewport", "warp_mouse");
}

Viewport *Viewport::_new()
{
	return (Viewport *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Viewport")());
}
void Viewport::_gui_remove_focus() {
	___godot_icall_void(___mb.mb__gui_remove_focus, (const Object *) this);
}

void Viewport::_gui_show_tooltip() {
	___godot_icall_void(___mb.mb__gui_show_tooltip, (const Object *) this);
}

void Viewport::_own_world_changed() {
	___godot_icall_void(___mb.mb__own_world_changed, (const Object *) this);
}

void Viewport::_post_gui_grab_click_focus() {
	___godot_icall_void(___mb.mb__post_gui_grab_click_focus, (const Object *) this);
}

void Viewport::_subwindow_visibility_changed() {
	___godot_icall_void(___mb.mb__subwindow_visibility_changed, (const Object *) this);
}

void Viewport::_vp_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__vp_input, (const Object *) this, arg0.ptr());
}

void Viewport::_vp_input_text(const String text) {
	___godot_icall_void_String(___mb.mb__vp_input_text, (const Object *) this, text);
}

void Viewport::_vp_unhandled_input(const Ref<InputEvent> arg0) {
	___godot_icall_void_Object(___mb.mb__vp_unhandled_input, (const Object *) this, arg0.ptr());
}

Ref<World> Viewport::find_world() const {
	return Ref<World>::__internal_constructor(___godot_icall_Object(___mb.mb_find_world, (const Object *) this));
}

Ref<World2D> Viewport::find_world_2d() const {
	return Ref<World2D>::__internal_constructor(___godot_icall_Object(___mb.mb_find_world_2d, (const Object *) this));
}

Camera *Viewport::get_camera() const {
	return (Camera *) ___godot_icall_Object(___mb.mb_get_camera, (const Object *) this);
}

Transform2D Viewport::get_canvas_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_canvas_transform, (const Object *) this);
}

Viewport::ClearMode Viewport::get_clear_mode() const {
	return (Viewport::ClearMode) ___godot_icall_int(___mb.mb_get_clear_mode, (const Object *) this);
}

Viewport::DebugDraw Viewport::get_debug_draw() const {
	return (Viewport::DebugDraw) ___godot_icall_int(___mb.mb_get_debug_draw, (const Object *) this);
}

Transform2D Viewport::get_final_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_final_transform, (const Object *) this);
}

Transform2D Viewport::get_global_canvas_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_global_canvas_transform, (const Object *) this);
}

bool Viewport::get_hdr() const {
	return ___godot_icall_bool(___mb.mb_get_hdr, (const Object *) this);
}

bool Viewport::get_keep_3d_linear() const {
	return ___godot_icall_bool(___mb.mb_get_keep_3d_linear, (const Object *) this);
}

Control *Viewport::get_modal_stack_top() const {
	return (Control *) ___godot_icall_Object(___mb.mb_get_modal_stack_top, (const Object *) this);
}

Vector2 Viewport::get_mouse_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_mouse_position, (const Object *) this);
}

Viewport::MSAA Viewport::get_msaa() const {
	return (Viewport::MSAA) ___godot_icall_int(___mb.mb_get_msaa, (const Object *) this);
}

bool Viewport::get_physics_object_picking() {
	return ___godot_icall_bool(___mb.mb_get_physics_object_picking, (const Object *) this);
}

int64_t Viewport::get_render_info(const int64_t info) {
	return ___godot_icall_int_int(___mb.mb_get_render_info, (const Object *) this, info);
}

Viewport::ShadowAtlasQuadrantSubdiv Viewport::get_shadow_atlas_quadrant_subdiv(const int64_t quadrant) const {
	return (Viewport::ShadowAtlasQuadrantSubdiv) ___godot_icall_int_int(___mb.mb_get_shadow_atlas_quadrant_subdiv, (const Object *) this, quadrant);
}

int64_t Viewport::get_shadow_atlas_size() const {
	return ___godot_icall_int(___mb.mb_get_shadow_atlas_size, (const Object *) this);
}

Vector2 Viewport::get_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_size, (const Object *) this);
}

Vector2 Viewport::get_size_override() const {
	return ___godot_icall_Vector2(___mb.mb_get_size_override, (const Object *) this);
}

Ref<ViewportTexture> Viewport::get_texture() const {
	return Ref<ViewportTexture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

Viewport::UpdateMode Viewport::get_update_mode() const {
	return (Viewport::UpdateMode) ___godot_icall_int(___mb.mb_get_update_mode, (const Object *) this);
}

Viewport::Usage Viewport::get_usage() const {
	return (Viewport::Usage) ___godot_icall_int(___mb.mb_get_usage, (const Object *) this);
}

bool Viewport::get_vflip() const {
	return ___godot_icall_bool(___mb.mb_get_vflip, (const Object *) this);
}

RID Viewport::get_viewport_rid() const {
	return ___godot_icall_RID(___mb.mb_get_viewport_rid, (const Object *) this);
}

Rect2 Viewport::get_visible_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_visible_rect, (const Object *) this);
}

Ref<World> Viewport::get_world() const {
	return Ref<World>::__internal_constructor(___godot_icall_Object(___mb.mb_get_world, (const Object *) this));
}

Ref<World2D> Viewport::get_world_2d() const {
	return Ref<World2D>::__internal_constructor(___godot_icall_Object(___mb.mb_get_world_2d, (const Object *) this));
}

Variant Viewport::gui_get_drag_data() const {
	return ___godot_icall_Variant(___mb.mb_gui_get_drag_data, (const Object *) this);
}

bool Viewport::gui_has_modal_stack() const {
	return ___godot_icall_bool(___mb.mb_gui_has_modal_stack, (const Object *) this);
}

bool Viewport::gui_is_dragging() const {
	return ___godot_icall_bool(___mb.mb_gui_is_dragging, (const Object *) this);
}

bool Viewport::has_transparent_background() const {
	return ___godot_icall_bool(___mb.mb_has_transparent_background, (const Object *) this);
}

void Viewport::input(const Ref<InputEvent> local_event) {
	___godot_icall_void_Object(___mb.mb_input, (const Object *) this, local_event.ptr());
}

bool Viewport::is_3d_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_3d_disabled, (const Object *) this);
}

bool Viewport::is_audio_listener() const {
	return ___godot_icall_bool(___mb.mb_is_audio_listener, (const Object *) this);
}

bool Viewport::is_audio_listener_2d() const {
	return ___godot_icall_bool(___mb.mb_is_audio_listener_2d, (const Object *) this);
}

bool Viewport::is_handling_input_locally() const {
	return ___godot_icall_bool(___mb.mb_is_handling_input_locally, (const Object *) this);
}

bool Viewport::is_input_disabled() const {
	return ___godot_icall_bool(___mb.mb_is_input_disabled, (const Object *) this);
}

bool Viewport::is_input_handled() const {
	return ___godot_icall_bool(___mb.mb_is_input_handled, (const Object *) this);
}

bool Viewport::is_size_override_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_size_override_enabled, (const Object *) this);
}

bool Viewport::is_size_override_stretch_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_size_override_stretch_enabled, (const Object *) this);
}

bool Viewport::is_snap_controls_to_pixels_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_snap_controls_to_pixels_enabled, (const Object *) this);
}

bool Viewport::is_using_own_world() const {
	return ___godot_icall_bool(___mb.mb_is_using_own_world, (const Object *) this);
}

bool Viewport::is_using_render_direct_to_screen() const {
	return ___godot_icall_bool(___mb.mb_is_using_render_direct_to_screen, (const Object *) this);
}

void Viewport::set_as_audio_listener(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_as_audio_listener, (const Object *) this, enable);
}

void Viewport::set_as_audio_listener_2d(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_as_audio_listener_2d, (const Object *) this, enable);
}

void Viewport::set_attach_to_screen_rect(const Rect2 rect) {
	___godot_icall_void_Rect2(___mb.mb_set_attach_to_screen_rect, (const Object *) this, rect);
}

void Viewport::set_canvas_transform(const Transform2D xform) {
	___godot_icall_void_Transform2D(___mb.mb_set_canvas_transform, (const Object *) this, xform);
}

void Viewport::set_clear_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_clear_mode, (const Object *) this, mode);
}

void Viewport::set_debug_draw(const int64_t debug_draw) {
	___godot_icall_void_int(___mb.mb_set_debug_draw, (const Object *) this, debug_draw);
}

void Viewport::set_disable_3d(const bool disable) {
	___godot_icall_void_bool(___mb.mb_set_disable_3d, (const Object *) this, disable);
}

void Viewport::set_disable_input(const bool disable) {
	___godot_icall_void_bool(___mb.mb_set_disable_input, (const Object *) this, disable);
}

void Viewport::set_global_canvas_transform(const Transform2D xform) {
	___godot_icall_void_Transform2D(___mb.mb_set_global_canvas_transform, (const Object *) this, xform);
}

void Viewport::set_handle_input_locally(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_handle_input_locally, (const Object *) this, enable);
}

void Viewport::set_hdr(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_hdr, (const Object *) this, enable);
}

void Viewport::set_input_as_handled() {
	___godot_icall_void(___mb.mb_set_input_as_handled, (const Object *) this);
}

void Viewport::set_keep_3d_linear(const bool keep_3d_linear) {
	___godot_icall_void_bool(___mb.mb_set_keep_3d_linear, (const Object *) this, keep_3d_linear);
}

void Viewport::set_msaa(const int64_t msaa) {
	___godot_icall_void_int(___mb.mb_set_msaa, (const Object *) this, msaa);
}

void Viewport::set_physics_object_picking(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_physics_object_picking, (const Object *) this, enable);
}

void Viewport::set_shadow_atlas_quadrant_subdiv(const int64_t quadrant, const int64_t subdiv) {
	___godot_icall_void_int_int(___mb.mb_set_shadow_atlas_quadrant_subdiv, (const Object *) this, quadrant, subdiv);
}

void Viewport::set_shadow_atlas_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_shadow_atlas_size, (const Object *) this, size);
}

void Viewport::set_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_size, (const Object *) this, size);
}

void Viewport::set_size_override(const bool enable, const Vector2 size, const Vector2 margin) {
	___godot_icall_void_bool_Vector2_Vector2(___mb.mb_set_size_override, (const Object *) this, enable, size, margin);
}

void Viewport::set_size_override_stretch(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_size_override_stretch, (const Object *) this, enabled);
}

void Viewport::set_snap_controls_to_pixels(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_snap_controls_to_pixels, (const Object *) this, enabled);
}

void Viewport::set_transparent_background(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_transparent_background, (const Object *) this, enable);
}

void Viewport::set_update_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_update_mode, (const Object *) this, mode);
}

void Viewport::set_usage(const int64_t usage) {
	___godot_icall_void_int(___mb.mb_set_usage, (const Object *) this, usage);
}

void Viewport::set_use_arvr(const bool use) {
	___godot_icall_void_bool(___mb.mb_set_use_arvr, (const Object *) this, use);
}

void Viewport::set_use_own_world(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_own_world, (const Object *) this, enable);
}

void Viewport::set_use_render_direct_to_screen(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_render_direct_to_screen, (const Object *) this, enable);
}

void Viewport::set_vflip(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_vflip, (const Object *) this, enable);
}

void Viewport::set_world(const Ref<World> world) {
	___godot_icall_void_Object(___mb.mb_set_world, (const Object *) this, world.ptr());
}

void Viewport::set_world_2d(const Ref<World2D> world_2d) {
	___godot_icall_void_Object(___mb.mb_set_world_2d, (const Object *) this, world_2d.ptr());
}

void Viewport::unhandled_input(const Ref<InputEvent> local_event) {
	___godot_icall_void_Object(___mb.mb_unhandled_input, (const Object *) this, local_event.ptr());
}

void Viewport::update_worlds() {
	___godot_icall_void(___mb.mb_update_worlds, (const Object *) this);
}

bool Viewport::use_arvr() {
	return ___godot_icall_bool(___mb.mb_use_arvr, (const Object *) this);
}

void Viewport::warp_mouse(const Vector2 to_position) {
	___godot_icall_void_Vector2(___mb.mb_warp_mouse, (const Object *) this, to_position);
}

}