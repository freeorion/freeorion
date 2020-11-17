#ifndef GODOT_CPP_STREAMTEXTURE_HPP
#define GODOT_CPP_STREAMTEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Texture.hpp"
namespace godot {


class StreamTexture : public Texture {
	struct ___method_bindings {
		godot_method_bind *mb_get_load_path;
		godot_method_bind *mb_load;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "StreamTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static StreamTexture *_new();

	// methods
	String get_load_path() const;
	Error load(const String path);

};

}

#endif