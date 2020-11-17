#ifndef GODOT_CPP_VISIBILITYENABLER2D_HPP
#define GODOT_CPP_VISIBILITYENABLER2D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "VisibilityNotifier2D.hpp"
namespace godot {

class Node;

class VisibilityEnabler2D : public VisibilityNotifier2D {
	struct ___method_bindings {
		godot_method_bind *mb__node_removed;
		godot_method_bind *mb_is_enabler_enabled;
		godot_method_bind *mb_set_enabler;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisibilityEnabler2D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Enabler {
		ENABLER_PAUSE_ANIMATIONS = 0,
		ENABLER_FREEZE_BODIES = 1,
		ENABLER_PAUSE_PARTICLES = 2,
		ENABLER_PARENT_PROCESS = 3,
		ENABLER_PARENT_PHYSICS_PROCESS = 4,
		ENABLER_PAUSE_ANIMATED_SPRITES = 5,
		ENABLER_MAX = 6,
	};

	// constants


	static VisibilityEnabler2D *_new();

	// methods
	void _node_removed(const Node *arg0);
	bool is_enabler_enabled(const int64_t enabler) const;
	void set_enabler(const int64_t enabler, const bool enabled);

};

}

#endif