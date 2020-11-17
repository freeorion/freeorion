#ifndef GODOT_CPP_LINE2D_HPP
#define GODOT_CPP_LINE2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Line2D.hpp"

#include "Node2D.hpp"
namespace godot {

class Curve;
class Gradient;
class Texture;

class Line2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__curve_changed;
		godot_method_bind *mb__gradient_changed;
		godot_method_bind *mb_add_point;
		godot_method_bind *mb_clear_points;
		godot_method_bind *mb_get_antialiased;
		godot_method_bind *mb_get_begin_cap_mode;
		godot_method_bind *mb_get_curve;
		godot_method_bind *mb_get_default_color;
		godot_method_bind *mb_get_end_cap_mode;
		godot_method_bind *mb_get_gradient;
		godot_method_bind *mb_get_joint_mode;
		godot_method_bind *mb_get_point_count;
		godot_method_bind *mb_get_point_position;
		godot_method_bind *mb_get_points;
		godot_method_bind *mb_get_round_precision;
		godot_method_bind *mb_get_sharp_limit;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_get_texture_mode;
		godot_method_bind *mb_get_width;
		godot_method_bind *mb_remove_point;
		godot_method_bind *mb_set_antialiased;
		godot_method_bind *mb_set_begin_cap_mode;
		godot_method_bind *mb_set_curve;
		godot_method_bind *mb_set_default_color;
		godot_method_bind *mb_set_end_cap_mode;
		godot_method_bind *mb_set_gradient;
		godot_method_bind *mb_set_joint_mode;
		godot_method_bind *mb_set_point_position;
		godot_method_bind *mb_set_points;
		godot_method_bind *mb_set_round_precision;
		godot_method_bind *mb_set_sharp_limit;
		godot_method_bind *mb_set_texture;
		godot_method_bind *mb_set_texture_mode;
		godot_method_bind *mb_set_width;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Line2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum LineTextureMode {
		LINE_TEXTURE_NONE = 0,
		LINE_TEXTURE_TILE = 1,
		LINE_TEXTURE_STRETCH = 2,
	};
	enum LineCapMode {
		LINE_CAP_NONE = 0,
		LINE_CAP_BOX = 1,
		LINE_CAP_ROUND = 2,
	};
	enum LineJointMode {
		LINE_JOINT_SHARP = 0,
		LINE_JOINT_BEVEL = 1,
		LINE_JOINT_ROUND = 2,
	};

	// constants


	static Line2D *_new();

	// methods
	void _curve_changed();
	void _gradient_changed();
	void add_point(const Vector2 position, const int64_t at_position = -1);
	void clear_points();
	bool get_antialiased() const;
	Line2D::LineCapMode get_begin_cap_mode() const;
	Ref<Curve> get_curve() const;
	Color get_default_color() const;
	Line2D::LineCapMode get_end_cap_mode() const;
	Ref<Gradient> get_gradient() const;
	Line2D::LineJointMode get_joint_mode() const;
	int64_t get_point_count() const;
	Vector2 get_point_position(const int64_t i) const;
	PoolVector2Array get_points() const;
	int64_t get_round_precision() const;
	real_t get_sharp_limit() const;
	Ref<Texture> get_texture() const;
	Line2D::LineTextureMode get_texture_mode() const;
	real_t get_width() const;
	void remove_point(const int64_t i);
	void set_antialiased(const bool antialiased);
	void set_begin_cap_mode(const int64_t mode);
	void set_curve(const Ref<Curve> curve);
	void set_default_color(const Color color);
	void set_end_cap_mode(const int64_t mode);
	void set_gradient(const Ref<Gradient> color);
	void set_joint_mode(const int64_t mode);
	void set_point_position(const int64_t i, const Vector2 position);
	void set_points(const PoolVector2Array points);
	void set_round_precision(const int64_t precision);
	void set_sharp_limit(const real_t limit);
	void set_texture(const Ref<Texture> texture);
	void set_texture_mode(const int64_t mode);
	void set_width(const real_t width);

};

}

#endif