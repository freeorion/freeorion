#ifndef GODOT_CPP_ANIMATIONNODEBLEND2_HPP
#define GODOT_CPP_ANIMATIONNODEBLEND2_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AnimationNode.hpp"
namespace godot {


class AnimationNodeBlend2 : public AnimationNode {
	struct ___method_bindings {
		godot_method_bind *mb_is_using_sync;
		godot_method_bind *mb_set_use_sync;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimationNodeBlend2"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AnimationNodeBlend2 *_new();

	// methods
	bool is_using_sync() const;
	void set_use_sync(const bool enable);

};

}

#endif