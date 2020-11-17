#ifndef GODOT_CPP_VISUALSCRIPTCOMMENT_HPP
#define GODOT_CPP_VISUALSCRIPTCOMMENT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptComment : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_description;
		godot_method_bind *mb_get_size;
		godot_method_bind *mb_get_title;
		godot_method_bind *mb_set_description;
		godot_method_bind *mb_set_size;
		godot_method_bind *mb_set_title;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptComment"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScriptComment *_new();

	// methods
	String get_description() const;
	Vector2 get_size() const;
	String get_title() const;
	void set_description(const String description);
	void set_size(const Vector2 size);
	void set_title(const String title);

};

}

#endif