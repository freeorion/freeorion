#ifndef GODOT_CPP_SKINREFERENCE_HPP
#define GODOT_CPP_SKINREFERENCE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Skin;

class SkinReference : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb__skin_changed;
		godot_method_bind *mb_get_skeleton;
		godot_method_bind *mb_get_skin;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SkinReference"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _skin_changed();
	RID get_skeleton() const;
	Ref<Skin> get_skin() const;

};

}

#endif