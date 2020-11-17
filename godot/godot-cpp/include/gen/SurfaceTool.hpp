#ifndef GODOT_CPP_SURFACETOOL_HPP
#define GODOT_CPP_SURFACETOOL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Mesh;
class ArrayMesh;
class Material;

class SurfaceTool : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_add_bones;
		godot_method_bind *mb_add_color;
		godot_method_bind *mb_add_index;
		godot_method_bind *mb_add_normal;
		godot_method_bind *mb_add_smooth_group;
		godot_method_bind *mb_add_tangent;
		godot_method_bind *mb_add_triangle_fan;
		godot_method_bind *mb_add_uv;
		godot_method_bind *mb_add_uv2;
		godot_method_bind *mb_add_vertex;
		godot_method_bind *mb_add_weights;
		godot_method_bind *mb_append_from;
		godot_method_bind *mb_begin;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_commit;
		godot_method_bind *mb_commit_to_arrays;
		godot_method_bind *mb_create_from;
		godot_method_bind *mb_create_from_blend_shape;
		godot_method_bind *mb_deindex;
		godot_method_bind *mb_generate_normals;
		godot_method_bind *mb_generate_tangents;
		godot_method_bind *mb_index;
		godot_method_bind *mb_set_material;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SurfaceTool"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static SurfaceTool *_new();

	// methods
	void add_bones(const PoolIntArray bones);
	void add_color(const Color color);
	void add_index(const int64_t index);
	void add_normal(const Vector3 normal);
	void add_smooth_group(const bool smooth);
	void add_tangent(const Plane tangent);
	void add_triangle_fan(const PoolVector3Array vertices, const PoolVector2Array uvs = PoolVector2Array(), const PoolColorArray colors = PoolColorArray(), const PoolVector2Array uv2s = PoolVector2Array(), const PoolVector3Array normals = PoolVector3Array(), const Array tangents = Array());
	void add_uv(const Vector2 uv);
	void add_uv2(const Vector2 uv2);
	void add_vertex(const Vector3 vertex);
	void add_weights(const PoolRealArray weights);
	void append_from(const Ref<Mesh> existing, const int64_t surface, const Transform transform);
	void begin(const int64_t primitive);
	void clear();
	Ref<ArrayMesh> commit(const Ref<ArrayMesh> existing = nullptr, const int64_t flags = 97280);
	Array commit_to_arrays();
	void create_from(const Ref<Mesh> existing, const int64_t surface);
	void create_from_blend_shape(const Ref<Mesh> existing, const int64_t surface, const String blend_shape);
	void deindex();
	void generate_normals(const bool flip = false);
	void generate_tangents();
	void index();
	void set_material(const Ref<Material> material);

};

}

#endif