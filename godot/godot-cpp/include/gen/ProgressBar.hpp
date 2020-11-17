#ifndef GODOT_CPP_PROGRESSBAR_HPP
#define GODOT_CPP_PROGRESSBAR_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Range.hpp"
namespace godot {


class ProgressBar : public Range {
	struct ___method_bindings {
		godot_method_bind *mb_is_percent_visible;
		godot_method_bind *mb_set_percent_visible;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ProgressBar"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ProgressBar *_new();

	// methods
	bool is_percent_visible() const;
	void set_percent_visible(const bool visible);

};

}

#endif