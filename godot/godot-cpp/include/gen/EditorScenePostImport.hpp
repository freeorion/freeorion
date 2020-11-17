#ifndef GODOT_CPP_EDITORSCENEPOSTIMPORT_HPP
#define GODOT_CPP_EDITORSCENEPOSTIMPORT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Object;

class EditorScenePostImport : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_source_file;
		godot_method_bind *mb_get_source_folder;
		godot_method_bind *mb_post_import;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorScenePostImport"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	String get_source_file() const;
	String get_source_folder() const;
	Object *post_import(const Object *scene);

};

}

#endif