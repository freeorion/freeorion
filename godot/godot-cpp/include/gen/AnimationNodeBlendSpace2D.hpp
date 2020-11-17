#ifndef GODOT_CPP_ANIMATIONNODEBLENDSPACE2D_HPP
#define GODOT_CPP_ANIMATIONNODEBLENDSPACE2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "AnimationNodeBlendSpace2D.hpp"

#include "AnimationRootNode.hpp"
namespace godot {

class AnimationRootNode;

class AnimationNodeBlendSpace2D : public AnimationRootNode {
	struct ___method_bindings {
		godot_method_bind *mb__add_blend_point;
		godot_method_bind *mb__get_triangles;
		godot_method_bind *mb__set_triangles;
		godot_method_bind *mb__tree_changed;
		godot_method_bind *mb__update_triangles;
		godot_method_bind *mb_add_blend_point;
		godot_method_bind *mb_add_triangle;
		godot_method_bind *mb_get_auto_triangles;
		godot_method_bind *mb_get_blend_mode;
		godot_method_bind *mb_get_blend_point_count;
		godot_method_bind *mb_get_blend_point_node;
		godot_method_bind *mb_get_blend_point_position;
		godot_method_bind *mb_get_max_space;
		godot_method_bind *mb_get_min_space;
		godot_method_bind *mb_get_snap;
		godot_method_bind *mb_get_triangle_count;
		godot_method_bind *mb_get_triangle_point;
		godot_method_bind *mb_get_x_label;
		godot_method_bind *mb_get_y_label;
		godot_method_bind *mb_remove_blend_point;
		godot_method_bind *mb_remove_triangle;
		godot_method_bind *mb_set_auto_triangles;
		godot_method_bind *mb_set_blend_mode;
		godot_method_bind *mb_set_blend_point_node;
		godot_method_bind *mb_set_blend_point_position;
		godot_method_bind *mb_set_max_space;
		godot_method_bind *mb_set_min_space;
		godot_method_bind *mb_set_snap;
		godot_method_bind *mb_set_x_label;
		godot_method_bind *mb_set_y_label;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimationNodeBlendSpace2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum BlendMode {
		BLEND_MODE_INTERPOLATED = 0,
		BLEND_MODE_DISCRETE = 1,
		BLEND_MODE_DISCRETE_CARRY = 2,
	};

	// constants


	static AnimationNodeBlendSpace2D *_new();

	// methods
	void _add_blend_point(const int64_t index, const Ref<AnimationRootNode> node);
	PoolIntArray _get_triangles() const;
	void _set_triangles(const PoolIntArray triangles);
	void _tree_changed();
	void _update_triangles();
	void add_blend_point(const Ref<AnimationRootNode> node, const Vector2 pos, const int64_t at_index = -1);
	void add_triangle(const int64_t x, const int64_t y, const int64_t z, const int64_t at_index = -1);
	bool get_auto_triangles() const;
	AnimationNodeBlendSpace2D::BlendMode get_blend_mode() const;
	int64_t get_blend_point_count() const;
	Ref<AnimationRootNode> get_blend_point_node(const int64_t point) const;
	Vector2 get_blend_point_position(const int64_t point) const;
	Vector2 get_max_space() const;
	Vector2 get_min_space() const;
	Vector2 get_snap() const;
	int64_t get_triangle_count() const;
	int64_t get_triangle_point(const int64_t triangle, const int64_t point);
	String get_x_label() const;
	String get_y_label() const;
	void remove_blend_point(const int64_t point);
	void remove_triangle(const int64_t triangle);
	void set_auto_triangles(const bool enable);
	void set_blend_mode(const int64_t mode);
	void set_blend_point_node(const int64_t point, const Ref<AnimationRootNode> node);
	void set_blend_point_position(const int64_t point, const Vector2 pos);
	void set_max_space(const Vector2 max_space);
	void set_min_space(const Vector2 min_space);
	void set_snap(const Vector2 snap);
	void set_x_label(const String text);
	void set_y_label(const String text);

};

}

#endif