#ifndef GODOT_CPP_STATICBODY2D_HPP
#define GODOT_CPP_STATICBODY2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "PhysicsBody2D.hpp"
namespace godot {

class PhysicsMaterial;

class StaticBody2D : public PhysicsBody2D {
	struct ___method_bindings {
		godot_method_bind *mb__reload_physics_characteristics;
		godot_method_bind *mb_get_bounce;
		godot_method_bind *mb_get_constant_angular_velocity;
		godot_method_bind *mb_get_constant_linear_velocity;
		godot_method_bind *mb_get_friction;
		godot_method_bind *mb_get_physics_material_override;
		godot_method_bind *mb_set_bounce;
		godot_method_bind *mb_set_constant_angular_velocity;
		godot_method_bind *mb_set_constant_linear_velocity;
		godot_method_bind *mb_set_friction;
		godot_method_bind *mb_set_physics_material_override;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "StaticBody2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static StaticBody2D *_new();

	// methods
	void _reload_physics_characteristics();
	real_t get_bounce() const;
	real_t get_constant_angular_velocity() const;
	Vector2 get_constant_linear_velocity() const;
	real_t get_friction() const;
	Ref<PhysicsMaterial> get_physics_material_override() const;
	void set_bounce(const real_t bounce);
	void set_constant_angular_velocity(const real_t vel);
	void set_constant_linear_velocity(const Vector2 vel);
	void set_friction(const real_t friction);
	void set_physics_material_override(const Ref<PhysicsMaterial> physics_material_override);

};

}

#endif