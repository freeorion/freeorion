#ifndef GODOT_CPP_SLIDER_HPP
#define GODOT_CPP_SLIDER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Range.hpp"
namespace godot {

class InputEvent;

class Slider : public Range {
	struct ___method_bindings {
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb_get_ticks;
		godot_method_bind *mb_get_ticks_on_borders;
		godot_method_bind *mb_is_editable;
		godot_method_bind *mb_is_scrollable;
		godot_method_bind *mb_set_editable;
		godot_method_bind *mb_set_scrollable;
		godot_method_bind *mb_set_ticks;
		godot_method_bind *mb_set_ticks_on_borders;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Slider"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	void _gui_input(const Ref<InputEvent> arg0);
	int64_t get_ticks() const;
	bool get_ticks_on_borders() const;
	bool is_editable() const;
	bool is_scrollable() const;
	void set_editable(const bool editable);
	void set_scrollable(const bool scrollable);
	void set_ticks(const int64_t count);
	void set_ticks_on_borders(const bool ticks_on_border);

};

}

#endif