#include "ProximityGroup.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ProximityGroup::___method_bindings ProximityGroup::___mb = {};

void ProximityGroup::___init_method_bindings() {
	___mb.mb__proximity_group_broadcast = godot::api->godot_method_bind_get_method("ProximityGroup", "_proximity_group_broadcast");
	___mb.mb_broadcast = godot::api->godot_method_bind_get_method("ProximityGroup", "broadcast");
	___mb.mb_get_dispatch_mode = godot::api->godot_method_bind_get_method("ProximityGroup", "get_dispatch_mode");
	___mb.mb_get_grid_radius = godot::api->godot_method_bind_get_method("ProximityGroup", "get_grid_radius");
	___mb.mb_get_group_name = godot::api->godot_method_bind_get_method("ProximityGroup", "get_group_name");
	___mb.mb_set_dispatch_mode = godot::api->godot_method_bind_get_method("ProximityGroup", "set_dispatch_mode");
	___mb.mb_set_grid_radius = godot::api->godot_method_bind_get_method("ProximityGroup", "set_grid_radius");
	___mb.mb_set_group_name = godot::api->godot_method_bind_get_method("ProximityGroup", "set_group_name");
}

ProximityGroup *ProximityGroup::_new()
{
	return (ProximityGroup *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ProximityGroup")());
}
void ProximityGroup::_proximity_group_broadcast(const String name, const Variant params) {
	___godot_icall_void_String_Variant(___mb.mb__proximity_group_broadcast, (const Object *) this, name, params);
}

void ProximityGroup::broadcast(const String name, const Variant parameters) {
	___godot_icall_void_String_Variant(___mb.mb_broadcast, (const Object *) this, name, parameters);
}

ProximityGroup::DispatchMode ProximityGroup::get_dispatch_mode() const {
	return (ProximityGroup::DispatchMode) ___godot_icall_int(___mb.mb_get_dispatch_mode, (const Object *) this);
}

Vector3 ProximityGroup::get_grid_radius() const {
	return ___godot_icall_Vector3(___mb.mb_get_grid_radius, (const Object *) this);
}

String ProximityGroup::get_group_name() const {
	return ___godot_icall_String(___mb.mb_get_group_name, (const Object *) this);
}

void ProximityGroup::set_dispatch_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_dispatch_mode, (const Object *) this, mode);
}

void ProximityGroup::set_grid_radius(const Vector3 radius) {
	___godot_icall_void_Vector3(___mb.mb_set_grid_radius, (const Object *) this, radius);
}

void ProximityGroup::set_group_name(const String name) {
	___godot_icall_void_String(___mb.mb_set_group_name, (const Object *) this, name);
}

}