#include "Font.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Font::___method_bindings Font::___mb = {};

void Font::___init_method_bindings() {
	___mb.mb_draw = godot::api->godot_method_bind_get_method("Font", "draw");
	___mb.mb_draw_char = godot::api->godot_method_bind_get_method("Font", "draw_char");
	___mb.mb_get_ascent = godot::api->godot_method_bind_get_method("Font", "get_ascent");
	___mb.mb_get_descent = godot::api->godot_method_bind_get_method("Font", "get_descent");
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("Font", "get_height");
	___mb.mb_get_string_size = godot::api->godot_method_bind_get_method("Font", "get_string_size");
	___mb.mb_get_wordwrap_string_size = godot::api->godot_method_bind_get_method("Font", "get_wordwrap_string_size");
	___mb.mb_has_outline = godot::api->godot_method_bind_get_method("Font", "has_outline");
	___mb.mb_is_distance_field_hint = godot::api->godot_method_bind_get_method("Font", "is_distance_field_hint");
	___mb.mb_update_changes = godot::api->godot_method_bind_get_method("Font", "update_changes");
}

void Font::draw(const RID canvas_item, const Vector2 position, const String string, const Color modulate, const int64_t clip_w, const Color outline_modulate) const {
	___godot_icall_void_RID_Vector2_String_Color_int_Color(___mb.mb_draw, (const Object *) this, canvas_item, position, string, modulate, clip_w, outline_modulate);
}

real_t Font::draw_char(const RID canvas_item, const Vector2 position, const int64_t _char, const int64_t next, const Color modulate, const bool outline) const {
	return ___godot_icall_float_RID_Vector2_int_int_Color_bool(___mb.mb_draw_char, (const Object *) this, canvas_item, position, _char, next, modulate, outline);
}

real_t Font::get_ascent() const {
	return ___godot_icall_float(___mb.mb_get_ascent, (const Object *) this);
}

real_t Font::get_descent() const {
	return ___godot_icall_float(___mb.mb_get_descent, (const Object *) this);
}

real_t Font::get_height() const {
	return ___godot_icall_float(___mb.mb_get_height, (const Object *) this);
}

Vector2 Font::get_string_size(const String string) const {
	return ___godot_icall_Vector2_String(___mb.mb_get_string_size, (const Object *) this, string);
}

Vector2 Font::get_wordwrap_string_size(const String string, const real_t width) const {
	return ___godot_icall_Vector2_String_float(___mb.mb_get_wordwrap_string_size, (const Object *) this, string, width);
}

bool Font::has_outline() const {
	return ___godot_icall_bool(___mb.mb_has_outline, (const Object *) this);
}

bool Font::is_distance_field_hint() const {
	return ___godot_icall_bool(___mb.mb_is_distance_field_hint, (const Object *) this);
}

void Font::update_changes() {
	___godot_icall_void(___mb.mb_update_changes, (const Object *) this);
}

}