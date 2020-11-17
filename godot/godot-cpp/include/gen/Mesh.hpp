#ifndef GODOT_CPP_MESH_HPP
#define GODOT_CPP_MESH_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class Shape;
class Mesh;
class TriangleMesh;
class Material;

class Mesh : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_create_convex_shape;
		godot_method_bind *mb_create_outline;
		godot_method_bind *mb_create_trimesh_shape;
		godot_method_bind *mb_generate_triangle_mesh;
		godot_method_bind *mb_get_aabb;
		godot_method_bind *mb_get_faces;
		godot_method_bind *mb_get_lightmap_size_hint;
		godot_method_bind *mb_get_surface_count;
		godot_method_bind *mb_set_lightmap_size_hint;
		godot_method_bind *mb_surface_get_arrays;
		godot_method_bind *mb_surface_get_blend_shape_arrays;
		godot_method_bind *mb_surface_get_material;
		godot_method_bind *mb_surface_set_material;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Mesh"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum BlendShapeMode {
		BLEND_SHAPE_MODE_NORMALIZED = 0,
		BLEND_SHAPE_MODE_RELATIVE = 1,
	};
	enum PrimitiveType {
		PRIMITIVE_POINTS = 0,
		PRIMITIVE_LINES = 1,
		PRIMITIVE_LINE_STRIP = 2,
		PRIMITIVE_LINE_LOOP = 3,
		PRIMITIVE_TRIANGLES = 4,
		PRIMITIVE_TRIANGLE_STRIP = 5,
		PRIMITIVE_TRIANGLE_FAN = 6,
	};
	enum ArrayFormat {
		ARRAY_FORMAT_VERTEX = 1,
		ARRAY_FORMAT_NORMAL = 2,
		ARRAY_FORMAT_TANGENT = 4,
		ARRAY_FORMAT_COLOR = 8,
		ARRAY_COMPRESS_BASE = 9,
		ARRAY_FORMAT_TEX_UV = 16,
		ARRAY_FORMAT_TEX_UV2 = 32,
		ARRAY_FORMAT_BONES = 64,
		ARRAY_FORMAT_WEIGHTS = 128,
		ARRAY_FORMAT_INDEX = 256,
		ARRAY_COMPRESS_VERTEX = 512,
		ARRAY_COMPRESS_NORMAL = 1024,
		ARRAY_COMPRESS_TANGENT = 2048,
		ARRAY_COMPRESS_COLOR = 4096,
		ARRAY_COMPRESS_TEX_UV = 8192,
		ARRAY_COMPRESS_TEX_UV2 = 16384,
		ARRAY_COMPRESS_BONES = 32768,
		ARRAY_COMPRESS_WEIGHTS = 65536,
		ARRAY_COMPRESS_DEFAULT = 97280,
		ARRAY_COMPRESS_INDEX = 131072,
		ARRAY_FLAG_USE_2D_VERTICES = 262144,
		ARRAY_FLAG_USE_16_BIT_BONES = 524288,
	};
	enum ArrayType {
		ARRAY_VERTEX = 0,
		ARRAY_NORMAL = 1,
		ARRAY_TANGENT = 2,
		ARRAY_COLOR = 3,
		ARRAY_TEX_UV = 4,
		ARRAY_TEX_UV2 = 5,
		ARRAY_BONES = 6,
		ARRAY_WEIGHTS = 7,
		ARRAY_INDEX = 8,
		ARRAY_MAX = 9,
	};

	// constants

	// methods
	Ref<Shape> create_convex_shape() const;
	Ref<Mesh> create_outline(const real_t margin) const;
	Ref<Shape> create_trimesh_shape() const;
	Ref<TriangleMesh> generate_triangle_mesh() const;
	AABB get_aabb() const;
	PoolVector3Array get_faces() const;
	Vector2 get_lightmap_size_hint() const;
	int64_t get_surface_count() const;
	void set_lightmap_size_hint(const Vector2 size);
	Array surface_get_arrays(const int64_t surf_idx) const;
	Array surface_get_blend_shape_arrays(const int64_t surf_idx) const;
	Ref<Material> surface_get_material(const int64_t surf_idx) const;
	void surface_set_material(const int64_t surf_idx, const Ref<Material> material);

};

}

#endif