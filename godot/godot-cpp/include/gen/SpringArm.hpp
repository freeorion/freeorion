#ifndef GODOT_CPP_SPRINGARM_HPP
#define GODOT_CPP_SPRINGARM_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {

class Shape;

class SpringArm : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_add_excluded_object;
		godot_method_bind *mb_clear_excluded_objects;
		godot_method_bind *mb_get_collision_mask;
		godot_method_bind *mb_get_hit_length;
		godot_method_bind *mb_get_length;
		godot_method_bind *mb_get_margin;
		godot_method_bind *mb_get_shape;
		godot_method_bind *mb_remove_excluded_object;
		godot_method_bind *mb_set_collision_mask;
		godot_method_bind *mb_set_length;
		godot_method_bind *mb_set_margin;
		godot_method_bind *mb_set_shape;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SpringArm"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static SpringArm *_new();

	// methods
	void add_excluded_object(const RID RID);
	void clear_excluded_objects();
	int64_t get_collision_mask();
	real_t get_hit_length();
	real_t get_length() const;
	real_t get_margin();
	Ref<Shape> get_shape() const;
	bool remove_excluded_object(const RID RID);
	void set_collision_mask(const int64_t mask);
	void set_length(const real_t length);
	void set_margin(const real_t margin);
	void set_shape(const Ref<Shape> shape);

};

}

#endif