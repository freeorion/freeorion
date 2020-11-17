#include "VisualScriptPreload.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"


namespace godot {


VisualScriptPreload::___method_bindings VisualScriptPreload::___mb = {};

void VisualScriptPreload::___init_method_bindings() {
	___mb.mb_get_preload = godot::api->godot_method_bind_get_method("VisualScriptPreload", "get_preload");
	___mb.mb_set_preload = godot::api->godot_method_bind_get_method("VisualScriptPreload", "set_preload");
}

VisualScriptPreload *VisualScriptPreload::_new()
{
	return (VisualScriptPreload *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptPreload")());
}
Ref<Resource> VisualScriptPreload::get_preload() const {
	return Ref<Resource>::__internal_constructor(___godot_icall_Object(___mb.mb_get_preload, (const Object *) this));
}

void VisualScriptPreload::set_preload(const Ref<Resource> resource) {
	___godot_icall_void_Object(___mb.mb_set_preload, (const Object *) this, resource.ptr());
}

}