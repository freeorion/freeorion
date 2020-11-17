#ifndef GODOT_CPP_PHYSICS2DSHAPEQUERYPARAMETERS_HPP
#define GODOT_CPP_PHYSICS2DSHAPEQUERYPARAMETERS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Resource;

class Physics2DShapeQueryParameters : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_collision_layer;
		godot_method_bind *mb_get_exclude;
		godot_method_bind *mb_get_margin;
		godot_method_bind *mb_get_motion;
		godot_method_bind *mb_get_shape_rid;
		godot_method_bind *mb_get_transform;
		godot_method_bind *mb_is_collide_with_areas_enabled;
		godot_method_bind *mb_is_collide_with_bodies_enabled;
		godot_method_bind *mb_set_collide_with_areas;
		godot_method_bind *mb_set_collide_with_bodies;
		godot_method_bind *mb_set_collision_layer;
		godot_method_bind *mb_set_exclude;
		godot_method_bind *mb_set_margin;
		godot_method_bind *mb_set_motion;
		godot_method_bind *mb_set_shape;
		godot_method_bind *mb_set_shape_rid;
		godot_method_bind *mb_set_transform;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Physics2DShapeQueryParameters"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Physics2DShapeQueryParameters *_new();

	// methods
	int64_t get_collision_layer() const;
	Array get_exclude() const;
	real_t get_margin() const;
	Vector2 get_motion() const;
	RID get_shape_rid() const;
	Transform2D get_transform() const;
	bool is_collide_with_areas_enabled() const;
	bool is_collide_with_bodies_enabled() const;
	void set_collide_with_areas(const bool enable);
	void set_collide_with_bodies(const bool enable);
	void set_collision_layer(const int64_t collision_layer);
	void set_exclude(const Array exclude);
	void set_margin(const real_t margin);
	void set_motion(const Vector2 motion);
	void set_shape(const Ref<Resource> shape);
	void set_shape_rid(const RID shape);
	void set_transform(const Transform2D transform);

};

}

#endif