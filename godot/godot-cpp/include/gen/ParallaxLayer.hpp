#ifndef GODOT_CPP_PARALLAXLAYER_HPP
#define GODOT_CPP_PARALLAXLAYER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {


class ParallaxLayer : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_mirroring;
		godot_method_bind *mb_get_motion_offset;
		godot_method_bind *mb_get_motion_scale;
		godot_method_bind *mb_set_mirroring;
		godot_method_bind *mb_set_motion_offset;
		godot_method_bind *mb_set_motion_scale;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ParallaxLayer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ParallaxLayer *_new();

	// methods
	Vector2 get_mirroring() const;
	Vector2 get_motion_offset() const;
	Vector2 get_motion_scale() const;
	void set_mirroring(const Vector2 mirror);
	void set_motion_offset(const Vector2 offset);
	void set_motion_scale(const Vector2 scale);

};

}

#endif