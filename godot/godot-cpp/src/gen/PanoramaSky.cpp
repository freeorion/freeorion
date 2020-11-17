#include "PanoramaSky.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


PanoramaSky::___method_bindings PanoramaSky::___mb = {};

void PanoramaSky::___init_method_bindings() {
	___mb.mb_get_panorama = godot::api->godot_method_bind_get_method("PanoramaSky", "get_panorama");
	___mb.mb_set_panorama = godot::api->godot_method_bind_get_method("PanoramaSky", "set_panorama");
}

PanoramaSky *PanoramaSky::_new()
{
	return (PanoramaSky *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PanoramaSky")());
}
Ref<Texture> PanoramaSky::get_panorama() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_panorama, (const Object *) this));
}

void PanoramaSky::set_panorama(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_panorama, (const Object *) this, texture.ptr());
}

}