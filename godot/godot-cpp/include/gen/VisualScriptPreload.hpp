#ifndef GODOT_CPP_VISUALSCRIPTPRELOAD_HPP
#define GODOT_CPP_VISUALSCRIPTPRELOAD_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "VisualScriptNode.hpp"
namespace godot {

class Resource;

class VisualScriptPreload : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_preload;
		godot_method_bind *mb_set_preload;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptPreload"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScriptPreload *_new();

	// methods
	Ref<Resource> get_preload() const;
	void set_preload(const Ref<Resource> resource);

};

}

#endif