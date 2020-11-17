#ifndef GODOT_CPP_NATIVESCRIPT_HPP
#define GODOT_CPP_NATIVESCRIPT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Script.hpp"
namespace godot {

class GDNativeLibrary;

class NativeScript : public Script {
	struct ___method_bindings {
		godot_method_bind *mb_get_class_documentation;
		godot_method_bind *mb_get_class_name;
		godot_method_bind *mb_get_library;
		godot_method_bind *mb_get_method_documentation;
		godot_method_bind *mb_get_property_documentation;
		godot_method_bind *mb_get_script_class_icon_path;
		godot_method_bind *mb_get_script_class_name;
		godot_method_bind *mb_get_signal_documentation;
		godot_method_bind *mb_new;
		godot_method_bind *mb_set_class_name;
		godot_method_bind *mb_set_library;
		godot_method_bind *mb_set_script_class_icon_path;
		godot_method_bind *mb_set_script_class_name;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "NativeScript"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static NativeScript *_new();

	// methods
	String get_class_documentation() const;
	String get_class_name() const;
	Ref<GDNativeLibrary> get_library() const;
	String get_method_documentation(const String method) const;
	String get_property_documentation(const String path) const;
	String get_script_class_icon_path() const;
	String get_script_class_name() const;
	String get_signal_documentation(const String signal_name) const;
	Variant new_(const Array& __var_args = Array());
	void set_class_name(const String class_name);
	void set_library(const Ref<GDNativeLibrary> library);
	void set_script_class_icon_path(const String icon_path);
	void set_script_class_name(const String class_name);
	template <class... Args> Variant new_(Args... args){
		return new_(Array::make(args...));
	}

};

}

#endif