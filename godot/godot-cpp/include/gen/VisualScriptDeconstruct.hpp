#ifndef GODOT_CPP_VISUALSCRIPTDECONSTRUCT_HPP
#define GODOT_CPP_VISUALSCRIPTDECONSTRUCT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Variant.hpp"

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptDeconstruct : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb__get_elem_cache;
		godot_method_bind *mb__set_elem_cache;
		godot_method_bind *mb_get_deconstruct_type;
		godot_method_bind *mb_set_deconstruct_type;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptDeconstruct"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static VisualScriptDeconstruct *_new();

	// methods
	Array _get_elem_cache() const;
	void _set_elem_cache(const Array _cache);
	Variant::Type get_deconstruct_type() const;
	void set_deconstruct_type(const int64_t type);

};

}

#endif