#ifndef GODOT_CPP_VIEWPORTTEXTURE_HPP
#define GODOT_CPP_VIEWPORTTEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Texture.hpp"
namespace godot {


class ViewportTexture : public Texture {
	struct ___method_bindings {
		godot_method_bind *mb_get_viewport_path_in_scene;
		godot_method_bind *mb_set_viewport_path_in_scene;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ViewportTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ViewportTexture *_new();

	// methods
	NodePath get_viewport_path_in_scene() const;
	void set_viewport_path_in_scene(const NodePath path);

};

}

#endif