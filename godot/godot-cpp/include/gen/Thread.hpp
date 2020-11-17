#ifndef GODOT_CPP_THREAD_HPP
#define GODOT_CPP_THREAD_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class Object;

class Thread : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_get_id;
		godot_method_bind *mb_is_active;
		godot_method_bind *mb_start;
		godot_method_bind *mb_wait_to_finish;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Thread"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Priority {
		PRIORITY_LOW = 0,
		PRIORITY_NORMAL = 1,
		PRIORITY_HIGH = 2,
	};

	// constants


	static Thread *_new();

	// methods
	String get_id() const;
	bool is_active() const;
	Error start(const Object *instance, const String method, const Variant userdata = Variant(), const int64_t priority = 1);
	Variant wait_to_finish();

};

}

#endif