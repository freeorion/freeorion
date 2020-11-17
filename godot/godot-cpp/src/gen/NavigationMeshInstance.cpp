#include "NavigationMeshInstance.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "NavigationMesh.hpp"


namespace godot {


NavigationMeshInstance::___method_bindings NavigationMeshInstance::___mb = {};

void NavigationMeshInstance::___init_method_bindings() {
	___mb.mb_get_navigation_mesh = godot::api->godot_method_bind_get_method("NavigationMeshInstance", "get_navigation_mesh");
	___mb.mb_is_enabled = godot::api->godot_method_bind_get_method("NavigationMeshInstance", "is_enabled");
	___mb.mb_set_enabled = godot::api->godot_method_bind_get_method("NavigationMeshInstance", "set_enabled");
	___mb.mb_set_navigation_mesh = godot::api->godot_method_bind_get_method("NavigationMeshInstance", "set_navigation_mesh");
}

NavigationMeshInstance *NavigationMeshInstance::_new()
{
	return (NavigationMeshInstance *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"NavigationMeshInstance")());
}
Ref<NavigationMesh> NavigationMeshInstance::get_navigation_mesh() const {
	return Ref<NavigationMesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_navigation_mesh, (const Object *) this));
}

bool NavigationMeshInstance::is_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_enabled, (const Object *) this);
}

void NavigationMeshInstance::set_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_enabled, (const Object *) this, enabled);
}

void NavigationMeshInstance::set_navigation_mesh(const Ref<NavigationMesh> navmesh) {
	___godot_icall_void_Object(___mb.mb_set_navigation_mesh, (const Object *) this, navmesh.ptr());
}

}