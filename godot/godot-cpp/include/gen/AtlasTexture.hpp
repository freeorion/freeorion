#ifndef GODOT_CPP_ATLASTEXTURE_HPP
#define GODOT_CPP_ATLASTEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Texture.hpp"
namespace godot {

class Texture;

class AtlasTexture : public Texture {
	struct ___method_bindings {
		godot_method_bind *mb_get_atlas;
		godot_method_bind *mb_get_margin;
		godot_method_bind *mb_get_region;
		godot_method_bind *mb_has_filter_clip;
		godot_method_bind *mb_set_atlas;
		godot_method_bind *mb_set_filter_clip;
		godot_method_bind *mb_set_margin;
		godot_method_bind *mb_set_region;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AtlasTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AtlasTexture *_new();

	// methods
	Ref<Texture> get_atlas() const;
	Rect2 get_margin() const;
	Rect2 get_region() const;
	bool has_filter_clip() const;
	void set_atlas(const Ref<Texture> atlas);
	void set_filter_clip(const bool enable);
	void set_margin(const Rect2 margin);
	void set_region(const Rect2 region);

};

}

#endif