#ifndef GODOT_CPP_VISIBILITYENABLER_HPP
#define GODOT_CPP_VISIBILITYENABLER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "VisibilityNotifier.hpp"
namespace godot {

class Node;

class VisibilityEnabler : public VisibilityNotifier {
	struct ___method_bindings {
		godot_method_bind *mb__node_removed;
		godot_method_bind *mb_is_enabler_enabled;
		godot_method_bind *mb_set_enabler;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisibilityEnabler"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Enabler {
		ENABLER_PAUSE_ANIMATIONS = 0,
		ENABLER_FREEZE_BODIES = 1,
		ENABLER_MAX = 2,
	};

	// constants


	static VisibilityEnabler *_new();

	// methods
	void _node_removed(const Node *arg0);
	bool is_enabler_enabled(const int64_t enabler) const;
	void set_enabler(const int64_t enabler, const bool enabled);

};

}

#endif