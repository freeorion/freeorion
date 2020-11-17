#include "CubeMap.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"


namespace godot {


CubeMap::___method_bindings CubeMap::___mb = {};

void CubeMap::___init_method_bindings() {
	___mb.mb_get_flags = godot::api->godot_method_bind_get_method("CubeMap", "get_flags");
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("CubeMap", "get_height");
	___mb.mb_get_lossy_storage_quality = godot::api->godot_method_bind_get_method("CubeMap", "get_lossy_storage_quality");
	___mb.mb_get_side = godot::api->godot_method_bind_get_method("CubeMap", "get_side");
	___mb.mb_get_storage = godot::api->godot_method_bind_get_method("CubeMap", "get_storage");
	___mb.mb_get_width = godot::api->godot_method_bind_get_method("CubeMap", "get_width");
	___mb.mb_set_flags = godot::api->godot_method_bind_get_method("CubeMap", "set_flags");
	___mb.mb_set_lossy_storage_quality = godot::api->godot_method_bind_get_method("CubeMap", "set_lossy_storage_quality");
	___mb.mb_set_side = godot::api->godot_method_bind_get_method("CubeMap", "set_side");
	___mb.mb_set_storage = godot::api->godot_method_bind_get_method("CubeMap", "set_storage");
}

CubeMap *CubeMap::_new()
{
	return (CubeMap *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CubeMap")());
}
int64_t CubeMap::get_flags() const {
	return ___godot_icall_int(___mb.mb_get_flags, (const Object *) this);
}

int64_t CubeMap::get_height() const {
	return ___godot_icall_int(___mb.mb_get_height, (const Object *) this);
}

real_t CubeMap::get_lossy_storage_quality() const {
	return ___godot_icall_float(___mb.mb_get_lossy_storage_quality, (const Object *) this);
}

Ref<Image> CubeMap::get_side(const int64_t side) const {
	return Ref<Image>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_side, (const Object *) this, side));
}

CubeMap::Storage CubeMap::get_storage() const {
	return (CubeMap::Storage) ___godot_icall_int(___mb.mb_get_storage, (const Object *) this);
}

int64_t CubeMap::get_width() const {
	return ___godot_icall_int(___mb.mb_get_width, (const Object *) this);
}

void CubeMap::set_flags(const int64_t flags) {
	___godot_icall_void_int(___mb.mb_set_flags, (const Object *) this, flags);
}

void CubeMap::set_lossy_storage_quality(const real_t quality) {
	___godot_icall_void_float(___mb.mb_set_lossy_storage_quality, (const Object *) this, quality);
}

void CubeMap::set_side(const int64_t side, const Ref<Image> image) {
	___godot_icall_void_int_Object(___mb.mb_set_side, (const Object *) this, side, image.ptr());
}

void CubeMap::set_storage(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_storage, (const Object *) this, mode);
}

}