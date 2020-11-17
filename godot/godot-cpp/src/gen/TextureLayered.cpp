#include "TextureLayered.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"


namespace godot {


TextureLayered::___method_bindings TextureLayered::___mb = {};

void TextureLayered::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("TextureLayered", "_get_data");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("TextureLayered", "_set_data");
	___mb.mb_create = godot::api->godot_method_bind_get_method("TextureLayered", "create");
	___mb.mb_get_depth = godot::api->godot_method_bind_get_method("TextureLayered", "get_depth");
	___mb.mb_get_flags = godot::api->godot_method_bind_get_method("TextureLayered", "get_flags");
	___mb.mb_get_format = godot::api->godot_method_bind_get_method("TextureLayered", "get_format");
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("TextureLayered", "get_height");
	___mb.mb_get_layer_data = godot::api->godot_method_bind_get_method("TextureLayered", "get_layer_data");
	___mb.mb_get_width = godot::api->godot_method_bind_get_method("TextureLayered", "get_width");
	___mb.mb_set_data_partial = godot::api->godot_method_bind_get_method("TextureLayered", "set_data_partial");
	___mb.mb_set_flags = godot::api->godot_method_bind_get_method("TextureLayered", "set_flags");
	___mb.mb_set_layer_data = godot::api->godot_method_bind_get_method("TextureLayered", "set_layer_data");
}

Dictionary TextureLayered::_get_data() const {
	return ___godot_icall_Dictionary(___mb.mb__get_data, (const Object *) this);
}

void TextureLayered::_set_data(const Dictionary data) {
	___godot_icall_void_Dictionary(___mb.mb__set_data, (const Object *) this, data);
}

void TextureLayered::create(const int64_t width, const int64_t height, const int64_t depth, const int64_t format, const int64_t flags) {
	___godot_icall_void_int_int_int_int_int(___mb.mb_create, (const Object *) this, width, height, depth, format, flags);
}

int64_t TextureLayered::get_depth() const {
	return ___godot_icall_int(___mb.mb_get_depth, (const Object *) this);
}

int64_t TextureLayered::get_flags() const {
	return ___godot_icall_int(___mb.mb_get_flags, (const Object *) this);
}

Image::Format TextureLayered::get_format() const {
	return (Image::Format) ___godot_icall_int(___mb.mb_get_format, (const Object *) this);
}

int64_t TextureLayered::get_height() const {
	return ___godot_icall_int(___mb.mb_get_height, (const Object *) this);
}

Ref<Image> TextureLayered::get_layer_data(const int64_t layer) const {
	return Ref<Image>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_layer_data, (const Object *) this, layer));
}

int64_t TextureLayered::get_width() const {
	return ___godot_icall_int(___mb.mb_get_width, (const Object *) this);
}

void TextureLayered::set_data_partial(const Ref<Image> image, const int64_t x_offset, const int64_t y_offset, const int64_t layer, const int64_t mipmap) {
	___godot_icall_void_Object_int_int_int_int(___mb.mb_set_data_partial, (const Object *) this, image.ptr(), x_offset, y_offset, layer, mipmap);
}

void TextureLayered::set_flags(const int64_t flags) {
	___godot_icall_void_int(___mb.mb_set_flags, (const Object *) this, flags);
}

void TextureLayered::set_layer_data(const Ref<Image> image, const int64_t layer) {
	___godot_icall_void_Object_int(___mb.mb_set_layer_data, (const Object *) this, image.ptr(), layer);
}

}