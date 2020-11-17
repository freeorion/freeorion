#include "NinePatchRect.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


NinePatchRect::___method_bindings NinePatchRect::___mb = {};

void NinePatchRect::___init_method_bindings() {
	___mb.mb_get_h_axis_stretch_mode = godot::api->godot_method_bind_get_method("NinePatchRect", "get_h_axis_stretch_mode");
	___mb.mb_get_patch_margin = godot::api->godot_method_bind_get_method("NinePatchRect", "get_patch_margin");
	___mb.mb_get_region_rect = godot::api->godot_method_bind_get_method("NinePatchRect", "get_region_rect");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("NinePatchRect", "get_texture");
	___mb.mb_get_v_axis_stretch_mode = godot::api->godot_method_bind_get_method("NinePatchRect", "get_v_axis_stretch_mode");
	___mb.mb_is_draw_center_enabled = godot::api->godot_method_bind_get_method("NinePatchRect", "is_draw_center_enabled");
	___mb.mb_set_draw_center = godot::api->godot_method_bind_get_method("NinePatchRect", "set_draw_center");
	___mb.mb_set_h_axis_stretch_mode = godot::api->godot_method_bind_get_method("NinePatchRect", "set_h_axis_stretch_mode");
	___mb.mb_set_patch_margin = godot::api->godot_method_bind_get_method("NinePatchRect", "set_patch_margin");
	___mb.mb_set_region_rect = godot::api->godot_method_bind_get_method("NinePatchRect", "set_region_rect");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("NinePatchRect", "set_texture");
	___mb.mb_set_v_axis_stretch_mode = godot::api->godot_method_bind_get_method("NinePatchRect", "set_v_axis_stretch_mode");
}

NinePatchRect *NinePatchRect::_new()
{
	return (NinePatchRect *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"NinePatchRect")());
}
NinePatchRect::AxisStretchMode NinePatchRect::get_h_axis_stretch_mode() const {
	return (NinePatchRect::AxisStretchMode) ___godot_icall_int(___mb.mb_get_h_axis_stretch_mode, (const Object *) this);
}

int64_t NinePatchRect::get_patch_margin(const int64_t margin) const {
	return ___godot_icall_int_int(___mb.mb_get_patch_margin, (const Object *) this, margin);
}

Rect2 NinePatchRect::get_region_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_region_rect, (const Object *) this);
}

Ref<Texture> NinePatchRect::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

NinePatchRect::AxisStretchMode NinePatchRect::get_v_axis_stretch_mode() const {
	return (NinePatchRect::AxisStretchMode) ___godot_icall_int(___mb.mb_get_v_axis_stretch_mode, (const Object *) this);
}

bool NinePatchRect::is_draw_center_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_draw_center_enabled, (const Object *) this);
}

void NinePatchRect::set_draw_center(const bool draw_center) {
	___godot_icall_void_bool(___mb.mb_set_draw_center, (const Object *) this, draw_center);
}

void NinePatchRect::set_h_axis_stretch_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_h_axis_stretch_mode, (const Object *) this, mode);
}

void NinePatchRect::set_patch_margin(const int64_t margin, const int64_t value) {
	___godot_icall_void_int_int(___mb.mb_set_patch_margin, (const Object *) this, margin, value);
}

void NinePatchRect::set_region_rect(const Rect2 rect) {
	___godot_icall_void_Rect2(___mb.mb_set_region_rect, (const Object *) this, rect);
}

void NinePatchRect::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void NinePatchRect::set_v_axis_stretch_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_v_axis_stretch_mode, (const Object *) this, mode);
}

}