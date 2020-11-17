#ifndef GODOT_CPP_CONFIRMATIONDIALOG_HPP
#define GODOT_CPP_CONFIRMATIONDIALOG_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "AcceptDialog.hpp"
namespace godot {

class Button;

class ConfirmationDialog : public AcceptDialog {
	struct ___method_bindings {
		godot_method_bind *mb_get_cancel;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ConfirmationDialog"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ConfirmationDialog *_new();

	// methods
	Button *get_cancel();

};

}

#endif