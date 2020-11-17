#include "CameraTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CameraTexture::___method_bindings CameraTexture::___mb = {};

void CameraTexture::___init_method_bindings() {
	___mb.mb_get_camera_active = godot::api->godot_method_bind_get_method("CameraTexture", "get_camera_active");
	___mb.mb_get_camera_feed_id = godot::api->godot_method_bind_get_method("CameraTexture", "get_camera_feed_id");
	___mb.mb_get_which_feed = godot::api->godot_method_bind_get_method("CameraTexture", "get_which_feed");
	___mb.mb_set_camera_active = godot::api->godot_method_bind_get_method("CameraTexture", "set_camera_active");
	___mb.mb_set_camera_feed_id = godot::api->godot_method_bind_get_method("CameraTexture", "set_camera_feed_id");
	___mb.mb_set_which_feed = godot::api->godot_method_bind_get_method("CameraTexture", "set_which_feed");
}

CameraTexture *CameraTexture::_new()
{
	return (CameraTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CameraTexture")());
}
bool CameraTexture::get_camera_active() const {
	return ___godot_icall_bool(___mb.mb_get_camera_active, (const Object *) this);
}

int64_t CameraTexture::get_camera_feed_id() const {
	return ___godot_icall_int(___mb.mb_get_camera_feed_id, (const Object *) this);
}

CameraServer::FeedImage CameraTexture::get_which_feed() const {
	return (CameraServer::FeedImage) ___godot_icall_int(___mb.mb_get_which_feed, (const Object *) this);
}

void CameraTexture::set_camera_active(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_camera_active, (const Object *) this, active);
}

void CameraTexture::set_camera_feed_id(const int64_t feed_id) {
	___godot_icall_void_int(___mb.mb_set_camera_feed_id, (const Object *) this, feed_id);
}

void CameraTexture::set_which_feed(const int64_t which_feed) {
	___godot_icall_void_int(___mb.mb_set_which_feed, (const Object *) this, which_feed);
}

}