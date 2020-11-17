#ifndef GODOT_CPP_MESHTEXTURE_HPP
#define GODOT_CPP_MESHTEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Texture.hpp"
namespace godot {

class Texture;
class Mesh;

class MeshTexture : public Texture {
	struct ___method_bindings {
		godot_method_bind *mb_get_base_texture;
		godot_method_bind *mb_get_image_size;
		godot_method_bind *mb_get_mesh;
		godot_method_bind *mb_set_base_texture;
		godot_method_bind *mb_set_image_size;
		godot_method_bind *mb_set_mesh;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "MeshTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static MeshTexture *_new();

	// methods
	Ref<Texture> get_base_texture() const;
	Vector2 get_image_size() const;
	Ref<Mesh> get_mesh() const;
	void set_base_texture(const Ref<Texture> texture);
	void set_image_size(const Vector2 size);
	void set_mesh(const Ref<Mesh> mesh);

};

}

#endif