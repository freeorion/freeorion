#include "CSGBox.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Material.hpp"


namespace godot {


CSGBox::___method_bindings CSGBox::___mb = {};

void CSGBox::___init_method_bindings() {
	___mb.mb_get_depth = godot::api->godot_method_bind_get_method("CSGBox", "get_depth");
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("CSGBox", "get_height");
	___mb.mb_get_material = godot::api->godot_method_bind_get_method("CSGBox", "get_material");
	___mb.mb_get_width = godot::api->godot_method_bind_get_method("CSGBox", "get_width");
	___mb.mb_set_depth = godot::api->godot_method_bind_get_method("CSGBox", "set_depth");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("CSGBox", "set_height");
	___mb.mb_set_material = godot::api->godot_method_bind_get_method("CSGBox", "set_material");
	___mb.mb_set_width = godot::api->godot_method_bind_get_method("CSGBox", "set_width");
}

CSGBox *CSGBox::_new()
{
	return (CSGBox *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CSGBox")());
}
real_t CSGBox::get_depth() const {
	return ___godot_icall_float(___mb.mb_get_depth, (const Object *) this);
}

real_t CSGBox::get_height() const {
	return ___godot_icall_float(___mb.mb_get_height, (const Object *) this);
}

Ref<Material> CSGBox::get_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_material, (const Object *) this));
}

real_t CSGBox::get_width() const {
	return ___godot_icall_float(___mb.mb_get_width, (const Object *) this);
}

void CSGBox::set_depth(const real_t depth) {
	___godot_icall_void_float(___mb.mb_set_depth, (const Object *) this, depth);
}

void CSGBox::set_height(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_height, (const Object *) this, height);
}

void CSGBox::set_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_material, (const Object *) this, material.ptr());
}

void CSGBox::set_width(const real_t width) {
	___godot_icall_void_float(___mb.mb_set_width, (const Object *) this, width);
}

}