#include "StyleBox.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "CanvasItem.hpp"


namespace godot {


StyleBox::___method_bindings StyleBox::___mb = {};

void StyleBox::___init_method_bindings() {
	___mb.mb_draw = godot::api->godot_method_bind_get_method("StyleBox", "draw");
	___mb.mb_get_center_size = godot::api->godot_method_bind_get_method("StyleBox", "get_center_size");
	___mb.mb_get_current_item_drawn = godot::api->godot_method_bind_get_method("StyleBox", "get_current_item_drawn");
	___mb.mb_get_default_margin = godot::api->godot_method_bind_get_method("StyleBox", "get_default_margin");
	___mb.mb_get_margin = godot::api->godot_method_bind_get_method("StyleBox", "get_margin");
	___mb.mb_get_minimum_size = godot::api->godot_method_bind_get_method("StyleBox", "get_minimum_size");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("StyleBox", "get_offset");
	___mb.mb_set_default_margin = godot::api->godot_method_bind_get_method("StyleBox", "set_default_margin");
	___mb.mb_test_mask = godot::api->godot_method_bind_get_method("StyleBox", "test_mask");
}

void StyleBox::draw(const RID canvas_item, const Rect2 rect) const {
	___godot_icall_void_RID_Rect2(___mb.mb_draw, (const Object *) this, canvas_item, rect);
}

Vector2 StyleBox::get_center_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_center_size, (const Object *) this);
}

CanvasItem *StyleBox::get_current_item_drawn() const {
	return (CanvasItem *) ___godot_icall_Object(___mb.mb_get_current_item_drawn, (const Object *) this);
}

real_t StyleBox::get_default_margin(const int64_t margin) const {
	return ___godot_icall_float_int(___mb.mb_get_default_margin, (const Object *) this, margin);
}

real_t StyleBox::get_margin(const int64_t margin) const {
	return ___godot_icall_float_int(___mb.mb_get_margin, (const Object *) this, margin);
}

Vector2 StyleBox::get_minimum_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_minimum_size, (const Object *) this);
}

Vector2 StyleBox::get_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_offset, (const Object *) this);
}

void StyleBox::set_default_margin(const int64_t margin, const real_t offset) {
	___godot_icall_void_int_float(___mb.mb_set_default_margin, (const Object *) this, margin, offset);
}

bool StyleBox::test_mask(const Vector2 point, const Rect2 rect) const {
	return ___godot_icall_bool_Vector2_Rect2(___mb.mb_test_mask, (const Object *) this, point, rect);
}

}