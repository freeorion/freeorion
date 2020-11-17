#include "Texture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "Image.hpp"


namespace godot {


Texture::___method_bindings Texture::___mb = {};

void Texture::___init_method_bindings() {
	___mb.mb_draw = godot::api->godot_method_bind_get_method("Texture", "draw");
	___mb.mb_draw_rect = godot::api->godot_method_bind_get_method("Texture", "draw_rect");
	___mb.mb_draw_rect_region = godot::api->godot_method_bind_get_method("Texture", "draw_rect_region");
	___mb.mb_get_data = godot::api->godot_method_bind_get_method("Texture", "get_data");
	___mb.mb_get_flags = godot::api->godot_method_bind_get_method("Texture", "get_flags");
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("Texture", "get_height");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("Texture", "get_size");
	___mb.mb_get_width = godot::api->godot_method_bind_get_method("Texture", "get_width");
	___mb.mb_has_alpha = godot::api->godot_method_bind_get_method("Texture", "has_alpha");
	___mb.mb_set_flags = godot::api->godot_method_bind_get_method("Texture", "set_flags");
}

void Texture::draw(const RID canvas_item, const Vector2 position, const Color modulate, const bool transpose, const Ref<Texture> normal_map) const {
	___godot_icall_void_RID_Vector2_Color_bool_Object(___mb.mb_draw, (const Object *) this, canvas_item, position, modulate, transpose, normal_map.ptr());
}

void Texture::draw_rect(const RID canvas_item, const Rect2 rect, const bool tile, const Color modulate, const bool transpose, const Ref<Texture> normal_map) const {
	___godot_icall_void_RID_Rect2_bool_Color_bool_Object(___mb.mb_draw_rect, (const Object *) this, canvas_item, rect, tile, modulate, transpose, normal_map.ptr());
}

void Texture::draw_rect_region(const RID canvas_item, const Rect2 rect, const Rect2 src_rect, const Color modulate, const bool transpose, const Ref<Texture> normal_map, const bool clip_uv) const {
	___godot_icall_void_RID_Rect2_Rect2_Color_bool_Object_bool(___mb.mb_draw_rect_region, (const Object *) this, canvas_item, rect, src_rect, modulate, transpose, normal_map.ptr(), clip_uv);
}

Ref<Image> Texture::get_data() const {
	return Ref<Image>::__internal_constructor(___godot_icall_Object(___mb.mb_get_data, (const Object *) this));
}

int64_t Texture::get_flags() const {
	return ___godot_icall_int(___mb.mb_get_flags, (const Object *) this);
}

int64_t Texture::get_height() const {
	return ___godot_icall_int(___mb.mb_get_height, (const Object *) this);
}

Vector2 Texture::get_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_size, (const Object *) this);
}

int64_t Texture::get_width() const {
	return ___godot_icall_int(___mb.mb_get_width, (const Object *) this);
}

bool Texture::has_alpha() const {
	return ___godot_icall_bool(___mb.mb_has_alpha, (const Object *) this);
}

void Texture::set_flags(const int64_t flags) {
	___godot_icall_void_int(___mb.mb_set_flags, (const Object *) this, flags);
}

}