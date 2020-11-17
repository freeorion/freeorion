#include "CameraFeed.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"


namespace godot {


CameraFeed::___method_bindings CameraFeed::___mb = {};

void CameraFeed::___init_method_bindings() {
	___mb.mb__allocate_texture = godot::api->godot_method_bind_get_method("CameraFeed", "_allocate_texture");
	___mb.mb__set_RGB_img = godot::api->godot_method_bind_get_method("CameraFeed", "_set_RGB_img");
	___mb.mb__set_YCbCr_img = godot::api->godot_method_bind_get_method("CameraFeed", "_set_YCbCr_img");
	___mb.mb__set_YCbCr_imgs = godot::api->godot_method_bind_get_method("CameraFeed", "_set_YCbCr_imgs");
	___mb.mb__set_name = godot::api->godot_method_bind_get_method("CameraFeed", "_set_name");
	___mb.mb__set_position = godot::api->godot_method_bind_get_method("CameraFeed", "_set_position");
	___mb.mb_get_id = godot::api->godot_method_bind_get_method("CameraFeed", "get_id");
	___mb.mb_get_name = godot::api->godot_method_bind_get_method("CameraFeed", "get_name");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("CameraFeed", "get_position");
	___mb.mb_get_transform = godot::api->godot_method_bind_get_method("CameraFeed", "get_transform");
	___mb.mb_is_active = godot::api->godot_method_bind_get_method("CameraFeed", "is_active");
	___mb.mb_set_active = godot::api->godot_method_bind_get_method("CameraFeed", "set_active");
	___mb.mb_set_transform = godot::api->godot_method_bind_get_method("CameraFeed", "set_transform");
}

CameraFeed *CameraFeed::_new()
{
	return (CameraFeed *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CameraFeed")());
}
void CameraFeed::_allocate_texture(const int64_t width, const int64_t height, const int64_t format, const int64_t texture_type, const int64_t data_type) {
	___godot_icall_void_int_int_int_int_int(___mb.mb__allocate_texture, (const Object *) this, width, height, format, texture_type, data_type);
}

void CameraFeed::_set_RGB_img(const Ref<Image> rgb_img) {
	___godot_icall_void_Object(___mb.mb__set_RGB_img, (const Object *) this, rgb_img.ptr());
}

void CameraFeed::_set_YCbCr_img(const Ref<Image> ycbcr_img) {
	___godot_icall_void_Object(___mb.mb__set_YCbCr_img, (const Object *) this, ycbcr_img.ptr());
}

void CameraFeed::_set_YCbCr_imgs(const Ref<Image> y_img, const Ref<Image> cbcr_img) {
	___godot_icall_void_Object_Object(___mb.mb__set_YCbCr_imgs, (const Object *) this, y_img.ptr(), cbcr_img.ptr());
}

void CameraFeed::_set_name(const String name) {
	___godot_icall_void_String(___mb.mb__set_name, (const Object *) this, name);
}

void CameraFeed::_set_position(const int64_t position) {
	___godot_icall_void_int(___mb.mb__set_position, (const Object *) this, position);
}

int64_t CameraFeed::get_id() const {
	return ___godot_icall_int(___mb.mb_get_id, (const Object *) this);
}

String CameraFeed::get_name() const {
	return ___godot_icall_String(___mb.mb_get_name, (const Object *) this);
}

CameraFeed::FeedPosition CameraFeed::get_position() const {
	return (CameraFeed::FeedPosition) ___godot_icall_int(___mb.mb_get_position, (const Object *) this);
}

Transform2D CameraFeed::get_transform() const {
	return ___godot_icall_Transform2D(___mb.mb_get_transform, (const Object *) this);
}

bool CameraFeed::is_active() const {
	return ___godot_icall_bool(___mb.mb_is_active, (const Object *) this);
}

void CameraFeed::set_active(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_active, (const Object *) this, active);
}

void CameraFeed::set_transform(const Transform2D transform) {
	___godot_icall_void_Transform2D(___mb.mb_set_transform, (const Object *) this, transform);
}

}