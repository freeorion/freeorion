#ifndef GODOT_CPP_PACKEDSCENE_HPP
#define GODOT_CPP_PACKEDSCENE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class SceneState;
class Node;

class PackedScene : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_bundled_scene;
		godot_method_bind *mb__set_bundled_scene;
		godot_method_bind *mb_can_instance;
		godot_method_bind *mb_get_state;
		godot_method_bind *mb_instance;
		godot_method_bind *mb_pack;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PackedScene"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum GenEditState {
		GEN_EDIT_STATE_DISABLED = 0,
		GEN_EDIT_STATE_INSTANCE = 1,
		GEN_EDIT_STATE_MAIN = 2,
	};

	// constants


	static PackedScene *_new();

	// methods
	Dictionary _get_bundled_scene() const;
	void _set_bundled_scene(const Dictionary arg0);
	bool can_instance() const;
	Ref<SceneState> get_state();
	Node *instance(const int64_t edit_state = 0) const;
	Error pack(const Node *path);

};

}

#endif