#ifndef GODOT_CPP_LISTENER_HPP
#define GODOT_CPP_LISTENER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Spatial.hpp"
namespace godot {


class Listener : public Spatial {
	struct ___method_bindings {
		godot_method_bind *mb_clear_current;
		godot_method_bind *mb_get_listener_transform;
		godot_method_bind *mb_is_current;
		godot_method_bind *mb_make_current;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Listener"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Listener *_new();

	// methods
	void clear_current();
	Transform get_listener_transform() const;
	bool is_current() const;
	void make_current();

};

}

#endif