#include "VisualScriptTypeCast.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptTypeCast::___method_bindings VisualScriptTypeCast::___mb = {};

void VisualScriptTypeCast::___init_method_bindings() {
	___mb.mb_get_base_script = godot::api->godot_method_bind_get_method("VisualScriptTypeCast", "get_base_script");
	___mb.mb_get_base_type = godot::api->godot_method_bind_get_method("VisualScriptTypeCast", "get_base_type");
	___mb.mb_set_base_script = godot::api->godot_method_bind_get_method("VisualScriptTypeCast", "set_base_script");
	___mb.mb_set_base_type = godot::api->godot_method_bind_get_method("VisualScriptTypeCast", "set_base_type");
}

VisualScriptTypeCast *VisualScriptTypeCast::_new()
{
	return (VisualScriptTypeCast *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptTypeCast")());
}
String VisualScriptTypeCast::get_base_script() const {
	return ___godot_icall_String(___mb.mb_get_base_script, (const Object *) this);
}

String VisualScriptTypeCast::get_base_type() const {
	return ___godot_icall_String(___mb.mb_get_base_type, (const Object *) this);
}

void VisualScriptTypeCast::set_base_script(const String path) {
	___godot_icall_void_String(___mb.mb_set_base_script, (const Object *) this, path);
}

void VisualScriptTypeCast::set_base_type(const String type) {
	___godot_icall_void_String(___mb.mb_set_base_type, (const Object *) this, type);
}

}