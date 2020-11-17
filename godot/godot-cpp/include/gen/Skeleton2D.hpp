#ifndef GODOT_CPP_SKELETON2D_HPP
#define GODOT_CPP_SKELETON2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {

class Bone2D;

class Skeleton2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__update_bone_setup;
		godot_method_bind *mb__update_transform;
		godot_method_bind *mb_get_bone;
		godot_method_bind *mb_get_bone_count;
		godot_method_bind *mb_get_skeleton;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Skeleton2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Skeleton2D *_new();

	// methods
	void _update_bone_setup();
	void _update_transform();
	Bone2D *get_bone(const int64_t idx);
	int64_t get_bone_count() const;
	RID get_skeleton() const;

};

}

#endif