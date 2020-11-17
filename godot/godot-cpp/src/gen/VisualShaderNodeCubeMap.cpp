#include "VisualShaderNodeCubeMap.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "CubeMap.hpp"


namespace godot {


VisualShaderNodeCubeMap::___method_bindings VisualShaderNodeCubeMap::___mb = {};

void VisualShaderNodeCubeMap::___init_method_bindings() {
	___mb.mb_get_cube_map = godot::api->godot_method_bind_get_method("VisualShaderNodeCubeMap", "get_cube_map");
	___mb.mb_get_source = godot::api->godot_method_bind_get_method("VisualShaderNodeCubeMap", "get_source");
	___mb.mb_get_texture_type = godot::api->godot_method_bind_get_method("VisualShaderNodeCubeMap", "get_texture_type");
	___mb.mb_set_cube_map = godot::api->godot_method_bind_get_method("VisualShaderNodeCubeMap", "set_cube_map");
	___mb.mb_set_source = godot::api->godot_method_bind_get_method("VisualShaderNodeCubeMap", "set_source");
	___mb.mb_set_texture_type = godot::api->godot_method_bind_get_method("VisualShaderNodeCubeMap", "set_texture_type");
}

VisualShaderNodeCubeMap *VisualShaderNodeCubeMap::_new()
{
	return (VisualShaderNodeCubeMap *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeCubeMap")());
}
Ref<CubeMap> VisualShaderNodeCubeMap::get_cube_map() const {
	return Ref<CubeMap>::__internal_constructor(___godot_icall_Object(___mb.mb_get_cube_map, (const Object *) this));
}

VisualShaderNodeCubeMap::Source VisualShaderNodeCubeMap::get_source() const {
	return (VisualShaderNodeCubeMap::Source) ___godot_icall_int(___mb.mb_get_source, (const Object *) this);
}

VisualShaderNodeCubeMap::TextureType VisualShaderNodeCubeMap::get_texture_type() const {
	return (VisualShaderNodeCubeMap::TextureType) ___godot_icall_int(___mb.mb_get_texture_type, (const Object *) this);
}

void VisualShaderNodeCubeMap::set_cube_map(const Ref<CubeMap> value) {
	___godot_icall_void_Object(___mb.mb_set_cube_map, (const Object *) this, value.ptr());
}

void VisualShaderNodeCubeMap::set_source(const int64_t value) {
	___godot_icall_void_int(___mb.mb_set_source, (const Object *) this, value);
}

void VisualShaderNodeCubeMap::set_texture_type(const int64_t value) {
	___godot_icall_void_int(___mb.mb_set_texture_type, (const Object *) this, value);
}

}