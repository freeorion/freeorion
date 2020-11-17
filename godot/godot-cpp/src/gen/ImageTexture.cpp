#include "ImageTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"


namespace godot {


ImageTexture::___method_bindings ImageTexture::___mb = {};

void ImageTexture::___init_method_bindings() {
	___mb.mb__reload_hook = godot::api->godot_method_bind_get_method("ImageTexture", "_reload_hook");
	___mb.mb_create = godot::api->godot_method_bind_get_method("ImageTexture", "create");
	___mb.mb_create_from_image = godot::api->godot_method_bind_get_method("ImageTexture", "create_from_image");
	___mb.mb_get_format = godot::api->godot_method_bind_get_method("ImageTexture", "get_format");
	___mb.mb_get_lossy_storage_quality = godot::api->godot_method_bind_get_method("ImageTexture", "get_lossy_storage_quality");
	___mb.mb_get_storage = godot::api->godot_method_bind_get_method("ImageTexture", "get_storage");
	___mb.mb_load = godot::api->godot_method_bind_get_method("ImageTexture", "load");
	___mb.mb_set_data = godot::api->godot_method_bind_get_method("ImageTexture", "set_data");
	___mb.mb_set_lossy_storage_quality = godot::api->godot_method_bind_get_method("ImageTexture", "set_lossy_storage_quality");
	___mb.mb_set_size_override = godot::api->godot_method_bind_get_method("ImageTexture", "set_size_override");
	___mb.mb_set_storage = godot::api->godot_method_bind_get_method("ImageTexture", "set_storage");
}

ImageTexture *ImageTexture::_new()
{
	return (ImageTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ImageTexture")());
}
void ImageTexture::_reload_hook(const RID rid) {
	___godot_icall_void_RID(___mb.mb__reload_hook, (const Object *) this, rid);
}

void ImageTexture::create(const int64_t width, const int64_t height, const int64_t format, const int64_t flags) {
	___godot_icall_void_int_int_int_int(___mb.mb_create, (const Object *) this, width, height, format, flags);
}

void ImageTexture::create_from_image(const Ref<Image> image, const int64_t flags) {
	___godot_icall_void_Object_int(___mb.mb_create_from_image, (const Object *) this, image.ptr(), flags);
}

Image::Format ImageTexture::get_format() const {
	return (Image::Format) ___godot_icall_int(___mb.mb_get_format, (const Object *) this);
}

real_t ImageTexture::get_lossy_storage_quality() const {
	return ___godot_icall_float(___mb.mb_get_lossy_storage_quality, (const Object *) this);
}

ImageTexture::Storage ImageTexture::get_storage() const {
	return (ImageTexture::Storage) ___godot_icall_int(___mb.mb_get_storage, (const Object *) this);
}

Error ImageTexture::load(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_load, (const Object *) this, path);
}

void ImageTexture::set_data(const Ref<Image> image) {
	___godot_icall_void_Object(___mb.mb_set_data, (const Object *) this, image.ptr());
}

void ImageTexture::set_lossy_storage_quality(const real_t quality) {
	___godot_icall_void_float(___mb.mb_set_lossy_storage_quality, (const Object *) this, quality);
}

void ImageTexture::set_size_override(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_size_override, (const Object *) this, size);
}

void ImageTexture::set_storage(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_storage, (const Object *) this, mode);
}

}