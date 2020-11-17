#ifndef GODOT_CPP_SKIN_HPP
#define GODOT_CPP_SKIN_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class Skin : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_add_bind;
		godot_method_bind *mb_clear_binds;
		godot_method_bind *mb_get_bind_bone;
		godot_method_bind *mb_get_bind_count;
		godot_method_bind *mb_get_bind_pose;
		godot_method_bind *mb_set_bind_bone;
		godot_method_bind *mb_set_bind_count;
		godot_method_bind *mb_set_bind_pose;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Skin"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Skin *_new();

	// methods
	void add_bind(const int64_t bone, const Transform pose);
	void clear_binds();
	int64_t get_bind_bone(const int64_t bind_index) const;
	int64_t get_bind_count() const;
	Transform get_bind_pose(const int64_t bind_index) const;
	void set_bind_bone(const int64_t bind_index, const int64_t bone);
	void set_bind_count(const int64_t bind_count);
	void set_bind_pose(const int64_t bind_index, const Transform pose);

};

}

#endif