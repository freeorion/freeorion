#ifndef GODOT_CPP_VISUALSHADERNODETEXTUREUNIFORM_HPP
#define GODOT_CPP_VISUALSHADERNODETEXTUREUNIFORM_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualShaderNodeTextureUniform.hpp"

#include "VisualShaderNodeUniform.hpp"
namespace godot {


class VisualShaderNodeTextureUniform : public VisualShaderNodeUniform {
	struct ___method_bindings {
		godot_method_bind *mb_get_color_default;
		godot_method_bind *mb_get_texture_type;
		godot_method_bind *mb_set_color_default;
		godot_method_bind *mb_set_texture_type;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualShaderNodeTextureUniform"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TextureType {
		TYPE_DATA = 0,
		TYPE_COLOR = 1,
		TYPE_NORMALMAP = 2,
		TYPE_ANISO = 3,
	};
	enum ColorDefault {
		COLOR_DEFAULT_WHITE = 0,
		COLOR_DEFAULT_BLACK = 1,
	};

	// constants


	static VisualShaderNodeTextureUniform *_new();

	// methods
	VisualShaderNodeTextureUniform::ColorDefault get_color_default() const;
	VisualShaderNodeTextureUniform::TextureType get_texture_type() const;
	void set_color_default(const int64_t type);
	void set_texture_type(const int64_t type);

};

}

#endif