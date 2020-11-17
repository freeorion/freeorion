#include "Sprite3D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


Sprite3D::___method_bindings Sprite3D::___mb = {};

void Sprite3D::___init_method_bindings() {
	___mb.mb_get_frame = godot::api->godot_method_bind_get_method("Sprite3D", "get_frame");
	___mb.mb_get_frame_coords = godot::api->godot_method_bind_get_method("Sprite3D", "get_frame_coords");
	___mb.mb_get_hframes = godot::api->godot_method_bind_get_method("Sprite3D", "get_hframes");
	___mb.mb_get_region_rect = godot::api->godot_method_bind_get_method("Sprite3D", "get_region_rect");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("Sprite3D", "get_texture");
	___mb.mb_get_vframes = godot::api->godot_method_bind_get_method("Sprite3D", "get_vframes");
	___mb.mb_is_region = godot::api->godot_method_bind_get_method("Sprite3D", "is_region");
	___mb.mb_set_frame = godot::api->godot_method_bind_get_method("Sprite3D", "set_frame");
	___mb.mb_set_frame_coords = godot::api->godot_method_bind_get_method("Sprite3D", "set_frame_coords");
	___mb.mb_set_hframes = godot::api->godot_method_bind_get_method("Sprite3D", "set_hframes");
	___mb.mb_set_region = godot::api->godot_method_bind_get_method("Sprite3D", "set_region");
	___mb.mb_set_region_rect = godot::api->godot_method_bind_get_method("Sprite3D", "set_region_rect");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("Sprite3D", "set_texture");
	___mb.mb_set_vframes = godot::api->godot_method_bind_get_method("Sprite3D", "set_vframes");
}

Sprite3D *Sprite3D::_new()
{
	return (Sprite3D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Sprite3D")());
}
int64_t Sprite3D::get_frame() const {
	return ___godot_icall_int(___mb.mb_get_frame, (const Object *) this);
}

Vector2 Sprite3D::get_frame_coords() const {
	return ___godot_icall_Vector2(___mb.mb_get_frame_coords, (const Object *) this);
}

int64_t Sprite3D::get_hframes() const {
	return ___godot_icall_int(___mb.mb_get_hframes, (const Object *) this);
}

Rect2 Sprite3D::get_region_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_region_rect, (const Object *) this);
}

Ref<Texture> Sprite3D::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

int64_t Sprite3D::get_vframes() const {
	return ___godot_icall_int(___mb.mb_get_vframes, (const Object *) this);
}

bool Sprite3D::is_region() const {
	return ___godot_icall_bool(___mb.mb_is_region, (const Object *) this);
}

void Sprite3D::set_frame(const int64_t frame) {
	___godot_icall_void_int(___mb.mb_set_frame, (const Object *) this, frame);
}

void Sprite3D::set_frame_coords(const Vector2 coords) {
	___godot_icall_void_Vector2(___mb.mb_set_frame_coords, (const Object *) this, coords);
}

void Sprite3D::set_hframes(const int64_t hframes) {
	___godot_icall_void_int(___mb.mb_set_hframes, (const Object *) this, hframes);
}

void Sprite3D::set_region(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_region, (const Object *) this, enabled);
}

void Sprite3D::set_region_rect(const Rect2 rect) {
	___godot_icall_void_Rect2(___mb.mb_set_region_rect, (const Object *) this, rect);
}

void Sprite3D::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void Sprite3D::set_vframes(const int64_t vframes) {
	___godot_icall_void_int(___mb.mb_set_vframes, (const Object *) this, vframes);
}

}