#ifndef GODOT_CPP_VISUALSCRIPTINPUTACTION_HPP
#define GODOT_CPP_VISUALSCRIPTINPUTACTION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualScriptInputAction.hpp"

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptInputAction : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_action_mode;
		godot_method_bind *mb_get_action_name;
		godot_method_bind *mb_set_action_mode;
		godot_method_bind *mb_set_action_name;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptInputAction"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Mode {
		MODE_PRESSED = 0,
		MODE_RELEASED = 1,
		MODE_JUST_PRESSED = 2,
		MODE_JUST_RELEASED = 3,
	};

	// constants


	static VisualScriptInputAction *_new();

	// methods
	VisualScriptInputAction::Mode get_action_mode() const;
	String get_action_name() const;
	void set_action_mode(const int64_t mode);
	void set_action_name(const String name);

};

}

#endif