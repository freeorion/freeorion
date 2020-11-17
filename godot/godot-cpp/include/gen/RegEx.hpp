#ifndef GODOT_CPP_REGEX_HPP
#define GODOT_CPP_REGEX_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class RegExMatch;

class RegEx : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_clear;
		godot_method_bind *mb_compile;
		godot_method_bind *mb_get_group_count;
		godot_method_bind *mb_get_names;
		godot_method_bind *mb_get_pattern;
		godot_method_bind *mb_is_valid;
		godot_method_bind *mb_search;
		godot_method_bind *mb_search_all;
		godot_method_bind *mb_sub;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "RegEx"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static RegEx *_new();

	// methods
	void clear();
	Error compile(const String pattern);
	int64_t get_group_count() const;
	Array get_names() const;
	String get_pattern() const;
	bool is_valid() const;
	Ref<RegExMatch> search(const String subject, const int64_t offset = 0, const int64_t end = -1) const;
	Array search_all(const String subject, const int64_t offset = 0, const int64_t end = -1) const;
	String sub(const String subject, const String replacement, const bool all = false, const int64_t offset = 0, const int64_t end = -1) const;

};

}

#endif