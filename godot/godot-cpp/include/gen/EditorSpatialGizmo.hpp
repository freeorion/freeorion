#ifndef GODOT_CPP_EDITORSPATIALGIZMO_HPP
#define GODOT_CPP_EDITORSPATIALGIZMO_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "SpatialGizmo.hpp"
namespace godot {

class TriangleMesh;
class Material;
class ArrayMesh;
class SkinReference;
class EditorSpatialGizmoPlugin;
class Spatial;
class Camera;
class Node;

class EditorSpatialGizmo : public SpatialGizmo {
	struct ___method_bindings {
		godot_method_bind *mb_add_collision_segments;
		godot_method_bind *mb_add_collision_triangles;
		godot_method_bind *mb_add_handles;
		godot_method_bind *mb_add_lines;
		godot_method_bind *mb_add_mesh;
		godot_method_bind *mb_add_unscaled_billboard;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_commit_handle;
		godot_method_bind *mb_get_handle_name;
		godot_method_bind *mb_get_handle_value;
		godot_method_bind *mb_get_plugin;
		godot_method_bind *mb_get_spatial_node;
		godot_method_bind *mb_is_handle_highlighted;
		godot_method_bind *mb_redraw;
		godot_method_bind *mb_set_handle;
		godot_method_bind *mb_set_hidden;
		godot_method_bind *mb_set_spatial_node;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorSpatialGizmo"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void add_collision_segments(const PoolVector3Array segments);
	void add_collision_triangles(const Ref<TriangleMesh> triangles);
	void add_handles(const PoolVector3Array handles, const Ref<Material> material, const bool billboard = false, const bool secondary = false);
	void add_lines(const PoolVector3Array lines, const Ref<Material> material, const bool billboard = false, const Color modulate = Color(1,1,1,1));
	void add_mesh(const Ref<ArrayMesh> mesh, const bool billboard = false, const Ref<SkinReference> skeleton = nullptr, const Ref<Material> material = nullptr);
	void add_unscaled_billboard(const Ref<Material> material, const real_t default_scale = 1, const Color modulate = Color(1,1,1,1));
	void clear();
	void commit_handle(const int64_t index, const Variant restore, const bool cancel);
	String get_handle_name(const int64_t index);
	Variant get_handle_value(const int64_t index);
	Ref<EditorSpatialGizmoPlugin> get_plugin() const;
	Spatial *get_spatial_node() const;
	bool is_handle_highlighted(const int64_t index);
	void redraw();
	void set_handle(const int64_t index, const Camera *camera, const Vector2 point);
	void set_hidden(const bool hidden);
	void set_spatial_node(const Node *node);

};

}

#endif