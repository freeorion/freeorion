#ifndef GODOT_CPP_RANGE_HPP
#define GODOT_CPP_RANGE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Control.hpp"
namespace godot {

class Node;

class Range : public Control {
	struct ___method_bindings {
		godot_method_bind *mb_get_as_ratio;
		godot_method_bind *mb_get_max;
		godot_method_bind *mb_get_min;
		godot_method_bind *mb_get_page;
		godot_method_bind *mb_get_step;
		godot_method_bind *mb_get_value;
		godot_method_bind *mb_is_greater_allowed;
		godot_method_bind *mb_is_lesser_allowed;
		godot_method_bind *mb_is_ratio_exp;
		godot_method_bind *mb_is_using_rounded_values;
		godot_method_bind *mb_set_allow_greater;
		godot_method_bind *mb_set_allow_lesser;
		godot_method_bind *mb_set_as_ratio;
		godot_method_bind *mb_set_exp_ratio;
		godot_method_bind *mb_set_max;
		godot_method_bind *mb_set_min;
		godot_method_bind *mb_set_page;
		godot_method_bind *mb_set_step;
		godot_method_bind *mb_set_use_rounded_values;
		godot_method_bind *mb_set_value;
		godot_method_bind *mb_share;
		godot_method_bind *mb_unshare;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Range"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants

	// methods
	real_t get_as_ratio() const;
	real_t get_max() const;
	real_t get_min() const;
	real_t get_page() const;
	real_t get_step() const;
	real_t get_value() const;
	bool is_greater_allowed() const;
	bool is_lesser_allowed() const;
	bool is_ratio_exp() const;
	bool is_using_rounded_values() const;
	void set_allow_greater(const bool allow);
	void set_allow_lesser(const bool allow);
	void set_as_ratio(const real_t value);
	void set_exp_ratio(const bool enabled);
	void set_max(const real_t maximum);
	void set_min(const real_t minimum);
	void set_page(const real_t pagesize);
	void set_step(const real_t step);
	void set_use_rounded_values(const bool enabled);
	void set_value(const real_t value);
	void share(const Node *with);
	void unshare();

};

}

#endif