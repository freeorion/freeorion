#ifndef GODOT_CPP_SHORTCUT_HPP
#define GODOT_CPP_SHORTCUT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class InputEvent;

class ShortCut : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_as_text;
		godot_method_bind *mb_get_shortcut;
		godot_method_bind *mb_is_shortcut;
		godot_method_bind *mb_is_valid;
		godot_method_bind *mb_set_shortcut;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ShortCut"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ShortCut *_new();

	// methods
	String get_as_text() const;
	Ref<InputEvent> get_shortcut() const;
	bool is_shortcut(const Ref<InputEvent> event) const;
	bool is_valid() const;
	void set_shortcut(const Ref<InputEvent> event);

};

}

#endif