#ifndef GODOT_CPP_CONTAINER_HPP
#define GODOT_CPP_CONTAINER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Control.hpp"
namespace godot {

class Control;

class Container : public Control {
	struct ___method_bindings {
		godot_method_bind *mb__child_minsize_changed;
		godot_method_bind *mb__sort_children;
		godot_method_bind *mb_fit_child_in_rect;
		godot_method_bind *mb_queue_sort;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Container"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants
	const static int NOTIFICATION_SORT_CHILDREN = 50;


	static Container *_new();

	// methods
	void _child_minsize_changed();
	void _sort_children();
	void fit_child_in_rect(const Control *child, const Rect2 rect);
	void queue_sort();

};

}

#endif