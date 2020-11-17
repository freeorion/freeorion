#ifndef GODOT_CPP_MULTIMESHINSTANCE2D_HPP
#define GODOT_CPP_MULTIMESHINSTANCE2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {

class MultiMesh;
class Texture;

class MultiMeshInstance2D : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb_get_multimesh;
		godot_method_bind *mb_get_normal_map;
		godot_method_bind *mb_get_texture;
		godot_method_bind *mb_set_multimesh;
		godot_method_bind *mb_set_normal_map;
		godot_method_bind *mb_set_texture;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "MultiMeshInstance2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static MultiMeshInstance2D *_new();

	// methods
	Ref<MultiMesh> get_multimesh() const;
	Ref<Texture> get_normal_map() const;
	Ref<Texture> get_texture() const;
	void set_multimesh(const Ref<MultiMesh> multimesh);
	void set_normal_map(const Ref<Texture> normal_map);
	void set_texture(const Ref<Texture> texture);

};

}

#endif