#ifndef GODOT_CPP_BOXCONTAINER_HPP
#define GODOT_CPP_BOXCONTAINER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "BoxContainer.hpp"

#include "Container.hpp"
namespace godot {


class BoxContainer : public Container {
	struct ___method_bindings {
		godot_method_bind *mb_add_spacer;
		godot_method_bind *mb_get_alignment;
		godot_method_bind *mb_set_alignment;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "BoxContainer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum AlignMode {
		ALIGN_BEGIN = 0,
		ALIGN_CENTER = 1,
		ALIGN_END = 2,
	};

	// constants

	// methods
	void add_spacer(const bool begin);
	BoxContainer::AlignMode get_alignment() const;
	void set_alignment(const int64_t alignment);

};

}

#endif