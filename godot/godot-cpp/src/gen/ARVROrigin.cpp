#include "ARVROrigin.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ARVROrigin::___method_bindings ARVROrigin::___mb = {};

void ARVROrigin::___init_method_bindings() {
	___mb.mb_get_world_scale = godot::api->godot_method_bind_get_method("ARVROrigin", "get_world_scale");
	___mb.mb_set_world_scale = godot::api->godot_method_bind_get_method("ARVROrigin", "set_world_scale");
}

ARVROrigin *ARVROrigin::_new()
{
	return (ARVROrigin *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ARVROrigin")());
}
real_t ARVROrigin::get_world_scale() const {
	return ___godot_icall_float(___mb.mb_get_world_scale, (const Object *) this);
}

void ARVROrigin::set_world_scale(const real_t world_scale) {
	___godot_icall_void_float(___mb.mb_set_world_scale, (const Object *) this, world_scale);
}

}