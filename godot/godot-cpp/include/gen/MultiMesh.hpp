#ifndef GODOT_CPP_MULTIMESH_HPP
#define GODOT_CPP_MULTIMESH_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "MultiMesh.hpp"

#include "Resource.hpp"
namespace godot {

class Mesh;

class MultiMesh : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_color_array;
		godot_method_bind *mb__get_custom_data_array;
		godot_method_bind *mb__get_transform_2d_array;
		godot_method_bind *mb__get_transform_array;
		godot_method_bind *mb__set_color_array;
		godot_method_bind *mb__set_custom_data_array;
		godot_method_bind *mb__set_transform_2d_array;
		godot_method_bind *mb__set_transform_array;
		godot_method_bind *mb_get_aabb;
		godot_method_bind *mb_get_color_format;
		godot_method_bind *mb_get_custom_data_format;
		godot_method_bind *mb_get_instance_color;
		godot_method_bind *mb_get_instance_count;
		godot_method_bind *mb_get_instance_custom_data;
		godot_method_bind *mb_get_instance_transform;
		godot_method_bind *mb_get_instance_transform_2d;
		godot_method_bind *mb_get_mesh;
		godot_method_bind *mb_get_transform_format;
		godot_method_bind *mb_get_visible_instance_count;
		godot_method_bind *mb_set_as_bulk_array;
		godot_method_bind *mb_set_color_format;
		godot_method_bind *mb_set_custom_data_format;
		godot_method_bind *mb_set_instance_color;
		godot_method_bind *mb_set_instance_count;
		godot_method_bind *mb_set_instance_custom_data;
		godot_method_bind *mb_set_instance_transform;
		godot_method_bind *mb_set_instance_transform_2d;
		godot_method_bind *mb_set_mesh;
		godot_method_bind *mb_set_transform_format;
		godot_method_bind *mb_set_visible_instance_count;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "MultiMesh"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TransformFormat {
		TRANSFORM_2D = 0,
		TRANSFORM_3D = 1,
	};
	enum CustomDataFormat {
		CUSTOM_DATA_NONE = 0,
		CUSTOM_DATA_8BIT = 1,
		CUSTOM_DATA_FLOAT = 2,
	};
	enum ColorFormat {
		COLOR_NONE = 0,
		COLOR_8BIT = 1,
		COLOR_FLOAT = 2,
	};

	// constants


	static MultiMesh *_new();

	// methods
	PoolColorArray _get_color_array() const;
	PoolColorArray _get_custom_data_array() const;
	PoolVector2Array _get_transform_2d_array() const;
	PoolVector3Array _get_transform_array() const;
	void _set_color_array(const PoolColorArray arg0);
	void _set_custom_data_array(const PoolColorArray arg0);
	void _set_transform_2d_array(const PoolVector2Array arg0);
	void _set_transform_array(const PoolVector3Array arg0);
	AABB get_aabb() const;
	MultiMesh::ColorFormat get_color_format() const;
	MultiMesh::CustomDataFormat get_custom_data_format() const;
	Color get_instance_color(const int64_t instance) const;
	int64_t get_instance_count() const;
	Color get_instance_custom_data(const int64_t instance) const;
	Transform get_instance_transform(const int64_t instance) const;
	Transform2D get_instance_transform_2d(const int64_t instance) const;
	Ref<Mesh> get_mesh() const;
	MultiMesh::TransformFormat get_transform_format() const;
	int64_t get_visible_instance_count() const;
	void set_as_bulk_array(const PoolRealArray array);
	void set_color_format(const int64_t format);
	void set_custom_data_format(const int64_t format);
	void set_instance_color(const int64_t instance, const Color color);
	void set_instance_count(const int64_t count);
	void set_instance_custom_data(const int64_t instance, const Color custom_data);
	void set_instance_transform(const int64_t instance, const Transform transform);
	void set_instance_transform_2d(const int64_t instance, const Transform2D transform);
	void set_mesh(const Ref<Mesh> mesh);
	void set_transform_format(const int64_t format);
	void set_visible_instance_count(const int64_t count);

};

}

#endif