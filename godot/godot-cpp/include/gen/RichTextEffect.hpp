#ifndef GODOT_CPP_RICHTEXTEFFECT_HPP
#define GODOT_CPP_RICHTEXTEFFECT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class CharFXTransform;

class RichTextEffect : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__process_custom_fx;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "RichTextEffect"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static RichTextEffect *_new();

	// methods
	bool _process_custom_fx(const Ref<CharFXTransform> char_fx);

};

}

#endif