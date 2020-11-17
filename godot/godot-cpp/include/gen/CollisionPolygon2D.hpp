#ifndef GODOT_CPP_COLLISIONPOLYGON2D_HPP
#define GODOT_CPP_COLLISIONPOLYGON2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "CollisionPolygon2D.hpp"

#include "Node2D.hpp"
namespace godot {


class CollisionPolygon2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_build_mode;
		godot_method_bind *mb_get_one_way_collision_margin;
		godot_method_bind *mb_get_polygon;
		godot_method_bind *mb_is_disabled;
		godot_method_bind *mb_is_one_way_collision_enabled;
		godot_method_bind *mb_set_build_mode;
		godot_method_bind *mb_set_disabled;
		godot_method_bind *mb_set_one_way_collision;
		godot_method_bind *mb_set_one_way_collision_margin;
		godot_method_bind *mb_set_polygon;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CollisionPolygon2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum BuildMode {
		BUILD_SOLIDS = 0,
		BUILD_SEGMENTS = 1,
	};

	// constants


	static CollisionPolygon2D *_new();

	// methods
	CollisionPolygon2D::BuildMode get_build_mode() const;
	real_t get_one_way_collision_margin() const;
	PoolVector2Array get_polygon() const;
	bool is_disabled() const;
	bool is_one_way_collision_enabled() const;
	void set_build_mode(const int64_t build_mode);
	void set_disabled(const bool disabled);
	void set_one_way_collision(const bool enabled);
	void set_one_way_collision_margin(const real_t margin);
	void set_polygon(const PoolVector2Array polygon);

};

}

#endif