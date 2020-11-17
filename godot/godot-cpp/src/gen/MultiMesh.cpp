#include "MultiMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Mesh.hpp"


namespace godot {


MultiMesh::___method_bindings MultiMesh::___mb = {};

void MultiMesh::___init_method_bindings() {
	___mb.mb__get_color_array = godot::api->godot_method_bind_get_method("MultiMesh", "_get_color_array");
	___mb.mb__get_custom_data_array = godot::api->godot_method_bind_get_method("MultiMesh", "_get_custom_data_array");
	___mb.mb__get_transform_2d_array = godot::api->godot_method_bind_get_method("MultiMesh", "_get_transform_2d_array");
	___mb.mb__get_transform_array = godot::api->godot_method_bind_get_method("MultiMesh", "_get_transform_array");
	___mb.mb__set_color_array = godot::api->godot_method_bind_get_method("MultiMesh", "_set_color_array");
	___mb.mb__set_custom_data_array = godot::api->godot_method_bind_get_method("MultiMesh", "_set_custom_data_array");
	___mb.mb__set_transform_2d_array = godot::api->godot_method_bind_get_method("MultiMesh", "_set_transform_2d_array");
	___mb.mb__set_transform_array = godot::api->godot_method_bind_get_method("MultiMesh", "_set_transform_array");
	___mb.mb_get_aabb = godot::api->godot_method_bind_get_method("MultiMesh", "get_aabb");
	___mb.mb_get_color_format = godot::api->godot_method_bind_get_method("MultiMesh", "get_color_format");
	___mb.mb_get_custom_data_format = godot::api->godot_method_bind_get_method("MultiMesh", "get_custom_data_format");
	___mb.mb_get_instance_color = godot::api->godot_method_bind_get_method("MultiMesh", "get_instance_color");
	___mb.mb_get_instance_count = godot::api->godot_method_bind_get_method("MultiMesh", "get_instance_count");
	___mb.mb_get_instance_custom_data = godot::api->godot_method_bind_get_method("MultiMesh", "get_instance_custom_data");
	___mb.mb_get_instance_transform = godot::api->godot_method_bind_get_method("MultiMesh", "get_instance_transform");
	___mb.mb_get_instance_transform_2d = godot::api->godot_method_bind_get_method("MultiMesh", "get_instance_transform_2d");
	___mb.mb_get_mesh = godot::api->godot_method_bind_get_method("MultiMesh", "get_mesh");
	___mb.mb_get_transform_format = godot::api->godot_method_bind_get_method("MultiMesh", "get_transform_format");
	___mb.mb_get_visible_instance_count = godot::api->godot_method_bind_get_method("MultiMesh", "get_visible_instance_count");
	___mb.mb_set_as_bulk_array = godot::api->godot_method_bind_get_method("MultiMesh", "set_as_bulk_array");
	___mb.mb_set_color_format = godot::api->godot_method_bind_get_method("MultiMesh", "set_color_format");
	___mb.mb_set_custom_data_format = godot::api->godot_method_bind_get_method("MultiMesh", "set_custom_data_format");
	___mb.mb_set_instance_color = godot::api->godot_method_bind_get_method("MultiMesh", "set_instance_color");
	___mb.mb_set_instance_count = godot::api->godot_method_bind_get_method("MultiMesh", "set_instance_count");
	___mb.mb_set_instance_custom_data = godot::api->godot_method_bind_get_method("MultiMesh", "set_instance_custom_data");
	___mb.mb_set_instance_transform = godot::api->godot_method_bind_get_method("MultiMesh", "set_instance_transform");
	___mb.mb_set_instance_transform_2d = godot::api->godot_method_bind_get_method("MultiMesh", "set_instance_transform_2d");
	___mb.mb_set_mesh = godot::api->godot_method_bind_get_method("MultiMesh", "set_mesh");
	___mb.mb_set_transform_format = godot::api->godot_method_bind_get_method("MultiMesh", "set_transform_format");
	___mb.mb_set_visible_instance_count = godot::api->godot_method_bind_get_method("MultiMesh", "set_visible_instance_count");
}

MultiMesh *MultiMesh::_new()
{
	return (MultiMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MultiMesh")());
}
PoolColorArray MultiMesh::_get_color_array() const {
	return ___godot_icall_PoolColorArray(___mb.mb__get_color_array, (const Object *) this);
}

PoolColorArray MultiMesh::_get_custom_data_array() const {
	return ___godot_icall_PoolColorArray(___mb.mb__get_custom_data_array, (const Object *) this);
}

PoolVector2Array MultiMesh::_get_transform_2d_array() const {
	return ___godot_icall_PoolVector2Array(___mb.mb__get_transform_2d_array, (const Object *) this);
}

PoolVector3Array MultiMesh::_get_transform_array() const {
	return ___godot_icall_PoolVector3Array(___mb.mb__get_transform_array, (const Object *) this);
}

void MultiMesh::_set_color_array(const PoolColorArray arg0) {
	___godot_icall_void_PoolColorArray(___mb.mb__set_color_array, (const Object *) this, arg0);
}

void MultiMesh::_set_custom_data_array(const PoolColorArray arg0) {
	___godot_icall_void_PoolColorArray(___mb.mb__set_custom_data_array, (const Object *) this, arg0);
}

void MultiMesh::_set_transform_2d_array(const PoolVector2Array arg0) {
	___godot_icall_void_PoolVector2Array(___mb.mb__set_transform_2d_array, (const Object *) this, arg0);
}

void MultiMesh::_set_transform_array(const PoolVector3Array arg0) {
	___godot_icall_void_PoolVector3Array(___mb.mb__set_transform_array, (const Object *) this, arg0);
}

AABB MultiMesh::get_aabb() const {
	return ___godot_icall_AABB(___mb.mb_get_aabb, (const Object *) this);
}

MultiMesh::ColorFormat MultiMesh::get_color_format() const {
	return (MultiMesh::ColorFormat) ___godot_icall_int(___mb.mb_get_color_format, (const Object *) this);
}

MultiMesh::CustomDataFormat MultiMesh::get_custom_data_format() const {
	return (MultiMesh::CustomDataFormat) ___godot_icall_int(___mb.mb_get_custom_data_format, (const Object *) this);
}

Color MultiMesh::get_instance_color(const int64_t instance) const {
	return ___godot_icall_Color_int(___mb.mb_get_instance_color, (const Object *) this, instance);
}

int64_t MultiMesh::get_instance_count() const {
	return ___godot_icall_int(___mb.mb_get_instance_count, (const Object *) this);
}

Color MultiMesh::get_instance_custom_data(const int64_t instance) const {
	return ___godot_icall_Color_int(___mb.mb_get_instance_custom_data, (const Object *) this, instance);
}

Transform MultiMesh::get_instance_transform(const int64_t instance) const {
	return ___godot_icall_Transform_int(___mb.mb_get_instance_transform, (const Object *) this, instance);
}

Transform2D MultiMesh::get_instance_transform_2d(const int64_t instance) const {
	return ___godot_icall_Transform2D_int(___mb.mb_get_instance_transform_2d, (const Object *) this, instance);
}

Ref<Mesh> MultiMesh::get_mesh() const {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object(___mb.mb_get_mesh, (const Object *) this));
}

MultiMesh::TransformFormat MultiMesh::get_transform_format() const {
	return (MultiMesh::TransformFormat) ___godot_icall_int(___mb.mb_get_transform_format, (const Object *) this);
}

int64_t MultiMesh::get_visible_instance_count() const {
	return ___godot_icall_int(___mb.mb_get_visible_instance_count, (const Object *) this);
}

void MultiMesh::set_as_bulk_array(const PoolRealArray array) {
	___godot_icall_void_PoolRealArray(___mb.mb_set_as_bulk_array, (const Object *) this, array);
}

void MultiMesh::set_color_format(const int64_t format) {
	___godot_icall_void_int(___mb.mb_set_color_format, (const Object *) this, format);
}

void MultiMesh::set_custom_data_format(const int64_t format) {
	___godot_icall_void_int(___mb.mb_set_custom_data_format, (const Object *) this, format);
}

void MultiMesh::set_instance_color(const int64_t instance, const Color color) {
	___godot_icall_void_int_Color(___mb.mb_set_instance_color, (const Object *) this, instance, color);
}

void MultiMesh::set_instance_count(const int64_t count) {
	___godot_icall_void_int(___mb.mb_set_instance_count, (const Object *) this, count);
}

void MultiMesh::set_instance_custom_data(const int64_t instance, const Color custom_data) {
	___godot_icall_void_int_Color(___mb.mb_set_instance_custom_data, (const Object *) this, instance, custom_data);
}

void MultiMesh::set_instance_transform(const int64_t instance, const Transform transform) {
	___godot_icall_void_int_Transform(___mb.mb_set_instance_transform, (const Object *) this, instance, transform);
}

void MultiMesh::set_instance_transform_2d(const int64_t instance, const Transform2D transform) {
	___godot_icall_void_int_Transform2D(___mb.mb_set_instance_transform_2d, (const Object *) this, instance, transform);
}

void MultiMesh::set_mesh(const Ref<Mesh> mesh) {
	___godot_icall_void_Object(___mb.mb_set_mesh, (const Object *) this, mesh.ptr());
}

void MultiMesh::set_transform_format(const int64_t format) {
	___godot_icall_void_int(___mb.mb_set_transform_format, (const Object *) this, format);
}

void MultiMesh::set_visible_instance_count(const int64_t count) {
	___godot_icall_void_int(___mb.mb_set_visible_instance_count, (const Object *) this, count);
}

}