#include "X509Certificate.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


X509Certificate::___method_bindings X509Certificate::___mb = {};

void X509Certificate::___init_method_bindings() {
	___mb.mb_load = godot::api->godot_method_bind_get_method("X509Certificate", "load");
	___mb.mb_save = godot::api->godot_method_bind_get_method("X509Certificate", "save");
}

X509Certificate *X509Certificate::_new()
{
	return (X509Certificate *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"X509Certificate")());
}
Error X509Certificate::load(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_load, (const Object *) this, path);
}

Error X509Certificate::save(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_save, (const Object *) this, path);
}

}