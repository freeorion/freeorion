#ifndef GODOT_CPP_X509CERTIFICATE_HPP
#define GODOT_CPP_X509CERTIFICATE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {


class X509Certificate : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_load;
		godot_method_bind *mb_save;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "X509Certificate"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static X509Certificate *_new();

	// methods
	Error load(const String path);
	Error save(const String path);

};

}

#endif