#ifndef GODOT_CPP_GDNATIVE_HPP
#define GODOT_CPP_GDNATIVE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class GDNativeLibrary;

class GDNative : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_call_native;
		godot_method_bind *mb_get_library;
		godot_method_bind *mb_initialize;
		godot_method_bind *mb_set_library;
		godot_method_bind *mb_terminate;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "GDNative"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static GDNative *_new();

	// methods
	Variant call_native(const String calling_type, const String procedure_name, const Array arguments);
	Ref<GDNativeLibrary> get_library() const;
	bool initialize();
	void set_library(const Ref<GDNativeLibrary> library);
	bool terminate();

};

}

#endif