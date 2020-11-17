#include "ArrayMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ArrayMesh::___method_bindings ArrayMesh::___mb = {};

void ArrayMesh::___init_method_bindings() {
	___mb.mb_add_blend_shape = godot::api->godot_method_bind_get_method("ArrayMesh", "add_blend_shape");
	___mb.mb_add_surface_from_arrays = godot::api->godot_method_bind_get_method("ArrayMesh", "add_surface_from_arrays");
	___mb.mb_clear_blend_shapes = godot::api->godot_method_bind_get_method("ArrayMesh", "clear_blend_shapes");
	___mb.mb_get_blend_shape_count = godot::api->godot_method_bind_get_method("ArrayMesh", "get_blend_shape_count");
	___mb.mb_get_blend_shape_mode = godot::api->godot_method_bind_get_method("ArrayMesh", "get_blend_shape_mode");
	___mb.mb_get_blend_shape_name = godot::api->godot_method_bind_get_method("ArrayMesh", "get_blend_shape_name");
	___mb.mb_get_custom_aabb = godot::api->godot_method_bind_get_method("ArrayMesh", "get_custom_aabb");
	___mb.mb_lightmap_unwrap = godot::api->godot_method_bind_get_method("ArrayMesh", "lightmap_unwrap");
	___mb.mb_regen_normalmaps = godot::api->godot_method_bind_get_method("ArrayMesh", "regen_normalmaps");
	___mb.mb_set_blend_shape_mode = godot::api->godot_method_bind_get_method("ArrayMesh", "set_blend_shape_mode");
	___mb.mb_set_custom_aabb = godot::api->godot_method_bind_get_method("ArrayMesh", "set_custom_aabb");
	___mb.mb_surface_find_by_name = godot::api->godot_method_bind_get_method("ArrayMesh", "surface_find_by_name");
	___mb.mb_surface_get_array_index_len = godot::api->godot_method_bind_get_method("ArrayMesh", "surface_get_array_index_len");
	___mb.mb_surface_get_array_len = godot::api->godot_method_bind_get_method("ArrayMesh", "surface_get_array_len");
	___mb.mb_surface_get_format = godot::api->godot_method_bind_get_method("ArrayMesh", "surface_get_format");
	___mb.mb_surface_get_name = godot::api->godot_method_bind_get_method("ArrayMesh", "surface_get_name");
	___mb.mb_surface_get_primitive_type = godot::api->godot_method_bind_get_method("ArrayMesh", "surface_get_primitive_type");
	___mb.mb_surface_remove = godot::api->godot_method_bind_get_method("ArrayMesh", "surface_remove");
	___mb.mb_surface_set_name = godot::api->godot_method_bind_get_method("ArrayMesh", "surface_set_name");
	___mb.mb_surface_update_region = godot::api->godot_method_bind_get_method("ArrayMesh", "surface_update_region");
}

ArrayMesh *ArrayMesh::_new()
{
	return (ArrayMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ArrayMesh")());
}
void ArrayMesh::add_blend_shape(const String name) {
	___godot_icall_void_String(___mb.mb_add_blend_shape, (const Object *) this, name);
}

void ArrayMesh::add_surface_from_arrays(const int64_t primitive, const Array arrays, const Array blend_shapes, const int64_t compress_flags) {
	___godot_icall_void_int_Array_Array_int(___mb.mb_add_surface_from_arrays, (const Object *) this, primitive, arrays, blend_shapes, compress_flags);
}

void ArrayMesh::clear_blend_shapes() {
	___godot_icall_void(___mb.mb_clear_blend_shapes, (const Object *) this);
}

int64_t ArrayMesh::get_blend_shape_count() const {
	return ___godot_icall_int(___mb.mb_get_blend_shape_count, (const Object *) this);
}

Mesh::BlendShapeMode ArrayMesh::get_blend_shape_mode() const {
	return (Mesh::BlendShapeMode) ___godot_icall_int(___mb.mb_get_blend_shape_mode, (const Object *) this);
}

String ArrayMesh::get_blend_shape_name(const int64_t index) const {
	return ___godot_icall_String_int(___mb.mb_get_blend_shape_name, (const Object *) this, index);
}

AABB ArrayMesh::get_custom_aabb() const {
	return ___godot_icall_AABB(___mb.mb_get_custom_aabb, (const Object *) this);
}

Error ArrayMesh::lightmap_unwrap(const Transform transform, const real_t texel_size) {
	return (Error) ___godot_icall_int_Transform_float(___mb.mb_lightmap_unwrap, (const Object *) this, transform, texel_size);
}

void ArrayMesh::regen_normalmaps() {
	___godot_icall_void(___mb.mb_regen_normalmaps, (const Object *) this);
}

void ArrayMesh::set_blend_shape_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_blend_shape_mode, (const Object *) this, mode);
}

void ArrayMesh::set_custom_aabb(const AABB aabb) {
	___godot_icall_void_AABB(___mb.mb_set_custom_aabb, (const Object *) this, aabb);
}

int64_t ArrayMesh::surface_find_by_name(const String name) const {
	return ___godot_icall_int_String(___mb.mb_surface_find_by_name, (const Object *) this, name);
}

int64_t ArrayMesh::surface_get_array_index_len(const int64_t surf_idx) const {
	return ___godot_icall_int_int(___mb.mb_surface_get_array_index_len, (const Object *) this, surf_idx);
}

int64_t ArrayMesh::surface_get_array_len(const int64_t surf_idx) const {
	return ___godot_icall_int_int(___mb.mb_surface_get_array_len, (const Object *) this, surf_idx);
}

int64_t ArrayMesh::surface_get_format(const int64_t surf_idx) const {
	return ___godot_icall_int_int(___mb.mb_surface_get_format, (const Object *) this, surf_idx);
}

String ArrayMesh::surface_get_name(const int64_t surf_idx) const {
	return ___godot_icall_String_int(___mb.mb_surface_get_name, (const Object *) this, surf_idx);
}

Mesh::PrimitiveType ArrayMesh::surface_get_primitive_type(const int64_t surf_idx) const {
	return (Mesh::PrimitiveType) ___godot_icall_int_int(___mb.mb_surface_get_primitive_type, (const Object *) this, surf_idx);
}

void ArrayMesh::surface_remove(const int64_t surf_idx) {
	___godot_icall_void_int(___mb.mb_surface_remove, (const Object *) this, surf_idx);
}

void ArrayMesh::surface_set_name(const int64_t surf_idx, const String name) {
	___godot_icall_void_int_String(___mb.mb_surface_set_name, (const Object *) this, surf_idx, name);
}

void ArrayMesh::surface_update_region(const int64_t surf_idx, const int64_t offset, const PoolByteArray data) {
	___godot_icall_void_int_int_PoolByteArray(___mb.mb_surface_update_region, (const Object *) this, surf_idx, offset, data);
}

}