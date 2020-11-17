#ifndef GODOT_CPP_EDITORRESOURCEPREVIEWGENERATOR_HPP
#define GODOT_CPP_EDITORRESOURCEPREVIEWGENERATOR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Texture;
class Resource;

class EditorResourcePreviewGenerator : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_can_generate_small_preview;
		godot_method_bind *mb_generate;
		godot_method_bind *mb_generate_from_path;
		godot_method_bind *mb_generate_small_preview_automatically;
		godot_method_bind *mb_handles;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "EditorResourcePreviewGenerator"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	bool can_generate_small_preview();
	Ref<Texture> generate(const Ref<Resource> from, const Vector2 size);
	Ref<Texture> generate_from_path(const String path, const Vector2 size);
	bool generate_small_preview_automatically();
	bool handles(const String type);

};

}

#endif