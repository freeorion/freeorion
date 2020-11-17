#ifndef GODOT_CPP_MESHDATATOOL_HPP
#define GODOT_CPP_MESHDATATOOL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class ArrayMesh;
class Material;

class MeshDataTool : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_clear;
		godot_method_bind *mb_commit_to_surface;
		godot_method_bind *mb_create_from_surface;
		godot_method_bind *mb_get_edge_count;
		godot_method_bind *mb_get_edge_faces;
		godot_method_bind *mb_get_edge_meta;
		godot_method_bind *mb_get_edge_vertex;
		godot_method_bind *mb_get_face_count;
		godot_method_bind *mb_get_face_edge;
		godot_method_bind *mb_get_face_meta;
		godot_method_bind *mb_get_face_normal;
		godot_method_bind *mb_get_face_vertex;
		godot_method_bind *mb_get_format;
		godot_method_bind *mb_get_material;
		godot_method_bind *mb_get_vertex;
		godot_method_bind *mb_get_vertex_bones;
		godot_method_bind *mb_get_vertex_color;
		godot_method_bind *mb_get_vertex_count;
		godot_method_bind *mb_get_vertex_edges;
		godot_method_bind *mb_get_vertex_faces;
		godot_method_bind *mb_get_vertex_meta;
		godot_method_bind *mb_get_vertex_normal;
		godot_method_bind *mb_get_vertex_tangent;
		godot_method_bind *mb_get_vertex_uv;
		godot_method_bind *mb_get_vertex_uv2;
		godot_method_bind *mb_get_vertex_weights;
		godot_method_bind *mb_set_edge_meta;
		godot_method_bind *mb_set_face_meta;
		godot_method_bind *mb_set_material;
		godot_method_bind *mb_set_vertex;
		godot_method_bind *mb_set_vertex_bones;
		godot_method_bind *mb_set_vertex_color;
		godot_method_bind *mb_set_vertex_meta;
		godot_method_bind *mb_set_vertex_normal;
		godot_method_bind *mb_set_vertex_tangent;
		godot_method_bind *mb_set_vertex_uv;
		godot_method_bind *mb_set_vertex_uv2;
		godot_method_bind *mb_set_vertex_weights;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "MeshDataTool"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static MeshDataTool *_new();

	// methods
	void clear();
	Error commit_to_surface(const Ref<ArrayMesh> mesh);
	Error create_from_surface(const Ref<ArrayMesh> mesh, const int64_t surface);
	int64_t get_edge_count() const;
	PoolIntArray get_edge_faces(const int64_t idx) const;
	Variant get_edge_meta(const int64_t idx) const;
	int64_t get_edge_vertex(const int64_t idx, const int64_t vertex) const;
	int64_t get_face_count() const;
	int64_t get_face_edge(const int64_t idx, const int64_t edge) const;
	Variant get_face_meta(const int64_t idx) const;
	Vector3 get_face_normal(const int64_t idx) const;
	int64_t get_face_vertex(const int64_t idx, const int64_t vertex) const;
	int64_t get_format() const;
	Ref<Material> get_material() const;
	Vector3 get_vertex(const int64_t idx) const;
	PoolIntArray get_vertex_bones(const int64_t idx) const;
	Color get_vertex_color(const int64_t idx) const;
	int64_t get_vertex_count() const;
	PoolIntArray get_vertex_edges(const int64_t idx) const;
	PoolIntArray get_vertex_faces(const int64_t idx) const;
	Variant get_vertex_meta(const int64_t idx) const;
	Vector3 get_vertex_normal(const int64_t idx) const;
	Plane get_vertex_tangent(const int64_t idx) const;
	Vector2 get_vertex_uv(const int64_t idx) const;
	Vector2 get_vertex_uv2(const int64_t idx) const;
	PoolRealArray get_vertex_weights(const int64_t idx) const;
	void set_edge_meta(const int64_t idx, const Variant meta);
	void set_face_meta(const int64_t idx, const Variant meta);
	void set_material(const Ref<Material> material);
	void set_vertex(const int64_t idx, const Vector3 vertex);
	void set_vertex_bones(const int64_t idx, const PoolIntArray bones);
	void set_vertex_color(const int64_t idx, const Color color);
	void set_vertex_meta(const int64_t idx, const Variant meta);
	void set_vertex_normal(const int64_t idx, const Vector3 normal);
	void set_vertex_tangent(const int64_t idx, const Plane tangent);
	void set_vertex_uv(const int64_t idx, const Vector2 uv);
	void set_vertex_uv2(const int64_t idx, const Vector2 uv2);
	void set_vertex_weights(const int64_t idx, const PoolRealArray weights);

};

}

#endif