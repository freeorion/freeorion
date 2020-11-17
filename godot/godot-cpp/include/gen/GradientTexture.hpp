#ifndef GODOT_CPP_GRADIENTTEXTURE_HPP
#define GODOT_CPP_GRADIENTTEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Texture.hpp"
namespace godot {

class Gradient;

class GradientTexture : public Texture {
	struct ___method_bindings {
		godot_method_bind *mb__update;
		godot_method_bind *mb_get_gradient;
		godot_method_bind *mb_set_gradient;
		godot_method_bind *mb_set_width;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "GradientTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static GradientTexture *_new();

	// methods
	void _update();
	Ref<Gradient> get_gradient() const;
	void set_gradient(const Ref<Gradient> gradient);
	void set_width(const int64_t width);

};

}

#endif