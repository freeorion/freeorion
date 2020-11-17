#include "BitMap.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"


namespace godot {


BitMap::___method_bindings BitMap::___mb = {};

void BitMap::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("BitMap", "_get_data");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("BitMap", "_set_data");
	___mb.mb_create = godot::api->godot_method_bind_get_method("BitMap", "create");
	___mb.mb_create_from_image_alpha = godot::api->godot_method_bind_get_method("BitMap", "create_from_image_alpha");
	___mb.mb_get_bit = godot::api->godot_method_bind_get_method("BitMap", "get_bit");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("BitMap", "get_size");
	___mb.mb_get_true_bit_count = godot::api->godot_method_bind_get_method("BitMap", "get_true_bit_count");
	___mb.mb_grow_mask = godot::api->godot_method_bind_get_method("BitMap", "grow_mask");
	___mb.mb_opaque_to_polygons = godot::api->godot_method_bind_get_method("BitMap", "opaque_to_polygons");
	___mb.mb_set_bit = godot::api->godot_method_bind_get_method("BitMap", "set_bit");
	___mb.mb_set_bit_rect = godot::api->godot_method_bind_get_method("BitMap", "set_bit_rect");
}

BitMap *BitMap::_new()
{
	return (BitMap *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"BitMap")());
}
Dictionary BitMap::_get_data() const {
	return ___godot_icall_Dictionary(___mb.mb__get_data, (const Object *) this);
}

void BitMap::_set_data(const Dictionary arg0) {
	___godot_icall_void_Dictionary(___mb.mb__set_data, (const Object *) this, arg0);
}

void BitMap::create(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_create, (const Object *) this, size);
}

void BitMap::create_from_image_alpha(const Ref<Image> image, const real_t threshold) {
	___godot_icall_void_Object_float(___mb.mb_create_from_image_alpha, (const Object *) this, image.ptr(), threshold);
}

bool BitMap::get_bit(const Vector2 position) const {
	return ___godot_icall_bool_Vector2(___mb.mb_get_bit, (const Object *) this, position);
}

Vector2 BitMap::get_size() const {
	return ___godot_icall_Vector2(___mb.mb_get_size, (const Object *) this);
}

int64_t BitMap::get_true_bit_count() const {
	return ___godot_icall_int(___mb.mb_get_true_bit_count, (const Object *) this);
}

void BitMap::grow_mask(const int64_t pixels, const Rect2 rect) {
	___godot_icall_void_int_Rect2(___mb.mb_grow_mask, (const Object *) this, pixels, rect);
}

Array BitMap::opaque_to_polygons(const Rect2 rect, const real_t epsilon) const {
	return ___godot_icall_Array_Rect2_float(___mb.mb_opaque_to_polygons, (const Object *) this, rect, epsilon);
}

void BitMap::set_bit(const Vector2 position, const bool bit) {
	___godot_icall_void_Vector2_bool(___mb.mb_set_bit, (const Object *) this, position, bit);
}

void BitMap::set_bit_rect(const Rect2 rect, const bool bit) {
	___godot_icall_void_Rect2_bool(___mb.mb_set_bit_rect, (const Object *) this, rect, bit);
}

}