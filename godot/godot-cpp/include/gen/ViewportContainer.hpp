#ifndef GODOT_CPP_VIEWPORTCONTAINER_HPP
#define GODOT_CPP_VIEWPORTCONTAINER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Container.hpp"
namespace godot {

class InputEvent;

class ViewportContainer : public Container {
	struct ___method_bindings {
		godot_method_bind *mb__input;
		godot_method_bind *mb__unhandled_input;
		godot_method_bind *mb_get_stretch_shrink;
		godot_method_bind *mb_is_stretch_enabled;
		godot_method_bind *mb_set_stretch;
		godot_method_bind *mb_set_stretch_shrink;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ViewportContainer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ViewportContainer *_new();

	// methods
	void _input(const Ref<InputEvent> event);
	void _unhandled_input(const Ref<InputEvent> event);
	int64_t get_stretch_shrink() const;
	bool is_stretch_enabled() const;
	void set_stretch(const bool enable);
	void set_stretch_shrink(const int64_t amount);

};

}

#endif