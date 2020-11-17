#include "StreamTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


StreamTexture::___method_bindings StreamTexture::___mb = {};

void StreamTexture::___init_method_bindings() {
	___mb.mb_get_load_path = godot::api->godot_method_bind_get_method("StreamTexture", "get_load_path");
	___mb.mb_load = godot::api->godot_method_bind_get_method("StreamTexture", "load");
}

StreamTexture *StreamTexture::_new()
{
	return (StreamTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"StreamTexture")());
}
String StreamTexture::get_load_path() const {
	return ___godot_icall_String(___mb.mb_get_load_path, (const Object *) this);
}

Error StreamTexture::load(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_load, (const Object *) this, path);
}

}