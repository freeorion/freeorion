#include "ARVRAnchor.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Mesh.hpp"


namespace godot {


ARVRAnchor::___method_bindings ARVRAnchor::___mb = {};

void ARVRAnchor::___init_method_bindings() {
	___mb.mb_get_anchor_id = godot::api->godot_method_bind_get_method("ARVRAnchor", "get_anchor_id");
	___mb.mb_get_anchor_name = godot::api->godot_method_bind_get_method("ARVRAnchor", "get_anchor_name");
	___mb.mb_get_is_active = godot::api->godot_method_bind_get_method("ARVRAnchor", "get_is_active");
	___mb.mb_get_mesh = godot::api->godot_method_bind_get_method("ARVRAnchor", "get_mesh");
	___mb.mb_get_plane = godot::api->godot_method_bind_get_method("ARVRAnchor", "get_plane");
	___mb.mb_get_size = godot::api->godot_method_bind_get_method("ARVRAnchor", "get_size");
	___mb.mb_set_anchor_id = godot::api->godot_method_bind_get_method("ARVRAnchor", "set_anchor_id");
}

ARVRAnchor *ARVRAnchor::_new()
{
	return (ARVRAnchor *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ARVRAnchor")());
}
int64_t ARVRAnchor::get_anchor_id() const {
	return ___godot_icall_int(___mb.mb_get_anchor_id, (const Object *) this);
}

String ARVRAnchor::get_anchor_name() const {
	return ___godot_icall_String(___mb.mb_get_anchor_name, (const Object *) this);
}

bool ARVRAnchor::get_is_active() const {
	return ___godot_icall_bool(___mb.mb_get_is_active, (const Object *) this);
}

Ref<Mesh> ARVRAnchor::get_mesh() const {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_mesh, (const Object *) this));
}

Plane ARVRAnchor::get_plane() const {
	return ___godot_icall_Plane(___mb.mb_get_plane, (const Object *) this);
}

Vector3 ARVRAnchor::get_size() const {
	return ___godot_icall_Vector3(___mb.mb_get_size, (const Object *) this);
}

void ARVRAnchor::set_anchor_id(const int64_t anchor_id) {
	___godot_icall_void_int(___mb.mb_set_anchor_id, (const Object *) this, anchor_id);
}

}