#include "VideoStreamWebm.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VideoStreamWebm::___method_bindings VideoStreamWebm::___mb = {};

void VideoStreamWebm::___init_method_bindings() {
	___mb.mb_get_file = godot::api->godot_method_bind_get_method("VideoStreamWebm", "get_file");
	___mb.mb_set_file = godot::api->godot_method_bind_get_method("VideoStreamWebm", "set_file");
}

VideoStreamWebm *VideoStreamWebm::_new()
{
	return (VideoStreamWebm *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VideoStreamWebm")());
}
String VideoStreamWebm::get_file() {
	return ___godot_icall_String(___mb.mb_get_file, (const Object *) this);
}

void VideoStreamWebm::set_file(const String file) {
	___godot_icall_void_String(___mb.mb_set_file, (const Object *) this, file);
}

}