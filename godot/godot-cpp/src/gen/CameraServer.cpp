#include "CameraServer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "CameraFeed.hpp"


namespace godot {


CameraServer *CameraServer::_singleton = NULL;


CameraServer::CameraServer() {
	_owner = godot::api->godot_global_get_singleton((char *) "CameraServer");
}


CameraServer::___method_bindings CameraServer::___mb = {};

void CameraServer::___init_method_bindings() {
	___mb.mb_add_feed = godot::api->godot_method_bind_get_method("CameraServer", "add_feed");
	___mb.mb_feeds = godot::api->godot_method_bind_get_method("CameraServer", "feeds");
	___mb.mb_get_feed = godot::api->godot_method_bind_get_method("CameraServer", "get_feed");
	___mb.mb_get_feed_count = godot::api->godot_method_bind_get_method("CameraServer", "get_feed_count");
	___mb.mb_remove_feed = godot::api->godot_method_bind_get_method("CameraServer", "remove_feed");
}

void CameraServer::add_feed(const Ref<CameraFeed> feed) {
	___godot_icall_void_Object(___mb.mb_add_feed, (const Object *) this, feed.ptr());
}

Array CameraServer::feeds() {
	return ___godot_icall_Array(___mb.mb_feeds, (const Object *) this);
}

Ref<CameraFeed> CameraServer::get_feed(const int64_t index) {
	return Ref<CameraFeed>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_feed, (const Object *) this, index));
}

int64_t CameraServer::get_feed_count() {
	return ___godot_icall_int(___mb.mb_get_feed_count, (const Object *) this);
}

void CameraServer::remove_feed(const Ref<CameraFeed> feed) {
	___godot_icall_void_Object(___mb.mb_remove_feed, (const Object *) this, feed.ptr());
}

}