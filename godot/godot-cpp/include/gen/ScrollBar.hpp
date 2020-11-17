#ifndef GODOT_CPP_SCROLLBAR_HPP
#define GODOT_CPP_SCROLLBAR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Range.hpp"
namespace godot {

class InputEvent;

class ScrollBar : public Range {
	struct ___method_bindings {
		godot_method_bind *mb__drag_node_exit;
		godot_method_bind *mb__drag_node_input;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb_get_custom_step;
		godot_method_bind *mb_set_custom_step;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ScrollBar"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _drag_node_exit();
	void _drag_node_input(const Ref<InputEvent> arg0);
	void _gui_input(const Ref<InputEvent> arg0);
	real_t get_custom_step() const;
	void set_custom_step(const real_t step);

};

}

#endif