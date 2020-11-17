#ifndef GODOT_CPP_SHADERMATERIAL_HPP
#define GODOT_CPP_SHADERMATERIAL_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Material.hpp"
namespace godot {

class Shader;

class ShaderMaterial : public Material {
	struct ___method_bindings {
		godot_method_bind *mb__shader_changed;
		godot_method_bind *mb_get_shader;
		godot_method_bind *mb_get_shader_param;
		godot_method_bind *mb_property_can_revert;
		godot_method_bind *mb_property_get_revert;
		godot_method_bind *mb_set_shader;
		godot_method_bind *mb_set_shader_param;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ShaderMaterial"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ShaderMaterial *_new();

	// methods
	void _shader_changed();
	Ref<Shader> get_shader() const;
	Variant get_shader_param(const String param) const;
	bool property_can_revert(const String name);
	Variant property_get_revert(const String name);
	void set_shader(const Ref<Shader> shader);
	void set_shader_param(const String param, const Variant value);

};

}

#endif