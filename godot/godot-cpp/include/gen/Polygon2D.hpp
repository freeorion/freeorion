#ifndef GODOT_CPP_POLYGON2D_HPP
#define GODOT_CPP_POLYGON2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {

class Texture;

class Polygon2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__get_bones;
		godot_method_bind *mb__set_bones;
		godot_method_bind *mb__skeleton_bone_setup_changed;
		godot_method_bind *mb_add_bone;
		godot_method_bind *mb_clear_bones;
		godot_method_bind *mb_erase_bone;
		godot_method_bind *mb_get_antialiased;
		godot_method_bind *mb_get_bone_count;
		godot_method_bind *mb_get_bone_path;
		godot_method_bind *mb_get_bone_weights;
		godot_method_bind *mb_get_color;
		godot_method_bind *mb_get_internal_vertex_count;
		godot_method_bind *mb_get_invert;
		godot_method_bind *mb_get_invert_border;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_get_polygon;
		godot_method_bind *mb_get_polygons;
		godot_method_bind *mb_get_skeleton;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_get_texture_offset;
		godot_method_bind *mb_get_texture_rotation;
		godot_method_bind *mb_get_texture_rotation_degrees;
		godot_method_bind *mb_get_texture_scale;
		godot_method_bind *mb_get_uv;
		godot_method_bind *mb_get_vertex_colors;
		godot_method_bind *mb_set_antialiased;
		godot_method_bind *mb_set_bone_path;
		godot_method_bind *mb_set_bone_weights;
		godot_method_bind *mb_set_color;
		godot_method_bind *mb_set_internal_vertex_count;
		godot_method_bind *mb_set_invert;
		godot_method_bind *mb_set_invert_border;
		godot_method_bind *mb_set_offset;
		godot_method_bind *mb_set_polygon;
		godot_method_bind *mb_set_polygons;
		godot_method_bind *mb_set_skeleton;
		godot_method_bind *mb_set_texture;
		godot_method_bind *mb_set_texture_offset;
		godot_method_bind *mb_set_texture_rotation;
		godot_method_bind *mb_set_texture_rotation_degrees;
		godot_method_bind *mb_set_texture_scale;
		godot_method_bind *mb_set_uv;
		godot_method_bind *mb_set_vertex_colors;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Polygon2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Polygon2D *_new();

	// methods
	Array _get_bones() const;
	void _set_bones(const Array bones);
	void _skeleton_bone_setup_changed();
	void add_bone(const NodePath path, const PoolRealArray weights);
	void clear_bones();
	void erase_bone(const int64_t index);
	bool get_antialiased() const;
	int64_t get_bone_count() const;
	NodePath get_bone_path(const int64_t index) const;
	PoolRealArray get_bone_weights(const int64_t index) const;
	Color get_color() const;
	int64_t get_internal_vertex_count() const;
	bool get_invert() const;
	real_t get_invert_border() const;
	Vector2 get_offset() const;
	PoolVector2Array get_polygon() const;
	Array get_polygons() const;
	NodePath get_skeleton() const;
	Ref<Texture> get_texture() const;
	Vector2 get_texture_offset() const;
	real_t get_texture_rotation() const;
	real_t get_texture_rotation_degrees() const;
	Vector2 get_texture_scale() const;
	PoolVector2Array get_uv() const;
	PoolColorArray get_vertex_colors() const;
	void set_antialiased(const bool antialiased);
	void set_bone_path(const int64_t index, const NodePath path);
	void set_bone_weights(const int64_t index, const PoolRealArray weights);
	void set_color(const Color color);
	void set_internal_vertex_count(const int64_t internal_vertex_count);
	void set_invert(const bool invert);
	void set_invert_border(const real_t invert_border);
	void set_offset(const Vector2 offset);
	void set_polygon(const PoolVector2Array polygon);
	void set_polygons(const Array polygons);
	void set_skeleton(const NodePath skeleton);
	void set_texture(const Ref<Texture> texture);
	void set_texture_offset(const Vector2 texture_offset);
	void set_texture_rotation(const real_t texture_rotation);
	void set_texture_rotation_degrees(const real_t texture_rotation);
	void set_texture_scale(const Vector2 texture_scale);
	void set_uv(const PoolVector2Array uv);
	void set_vertex_colors(const PoolColorArray vertex_colors);

};

}

#endif