#ifndef GODOT_CPP_CSGBOX_HPP
#define GODOT_CPP_CSGBOX_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "CSGPrimitive.hpp"
namespace godot {

class Material;

class CSGBox : public CSGPrimitive {
	struct ___method_bindings {
		godot_method_bind *mb_get_depth;
		godot_method_bind *mb_get_height;
		godot_method_bind *mb_get_material;
		godot_method_bind *mb_get_width;
		godot_method_bind *mb_set_depth;
		godot_method_bind *mb_set_height;
		godot_method_bind *mb_set_material;
		godot_method_bind *mb_set_width;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "CSGBox"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static CSGBox *_new();

	// methods
	real_t get_depth() const;
	real_t get_height() const;
	Ref<Material> get_material() const;
	real_t get_width() const;
	void set_depth(const real_t depth);
	void set_height(const real_t height);
	void set_material(const Ref<Material> material);
	void set_width(const real_t width);

};

}

#endif