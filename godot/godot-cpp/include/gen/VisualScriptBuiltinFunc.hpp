#ifndef GODOT_CPP_VISUALSCRIPTBUILTINFUNC_HPP
#define GODOT_CPP_VISUALSCRIPTBUILTINFUNC_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "VisualScriptBuiltinFunc.hpp"

#include "VisualScriptNode.hpp"
namespace godot {


class VisualScriptBuiltinFunc : public VisualScriptNode {
	struct ___method_bindings {
		godot_method_bind *mb_get_func;
		godot_method_bind *mb_set_func;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "VisualScriptBuiltinFunc"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum BuiltinFunc {
		MATH_SIN = 0,
		MATH_COS = 1,
		MATH_TAN = 2,
		MATH_SINH = 3,
		MATH_COSH = 4,
		MATH_TANH = 5,
		MATH_ASIN = 6,
		MATH_ACOS = 7,
		MATH_ATAN = 8,
		MATH_ATAN2 = 9,
		MATH_SQRT = 10,
		MATH_FMOD = 11,
		MATH_FPOSMOD = 12,
		MATH_FLOOR = 13,
		MATH_CEIL = 14,
		MATH_ROUND = 15,
		MATH_ABS = 16,
		MATH_SIGN = 17,
		MATH_POW = 18,
		MATH_LOG = 19,
		MATH_EXP = 20,
		MATH_ISNAN = 21,
		MATH_ISINF = 22,
		MATH_EASE = 23,
		MATH_DECIMALS = 24,
		MATH_STEPIFY = 25,
		MATH_LERP = 26,
		MATH_INVERSE_LERP = 27,
		MATH_RANGE_LERP = 28,
		MATH_MOVE_TOWARD = 29,
		MATH_DECTIME = 30,
		MATH_RANDOMIZE = 31,
		MATH_RAND = 32,
		MATH_RANDF = 33,
		MATH_RANDOM = 34,
		MATH_SEED = 35,
		MATH_RANDSEED = 36,
		MATH_DEG2RAD = 37,
		MATH_RAD2DEG = 38,
		MATH_LINEAR2DB = 39,
		MATH_DB2LINEAR = 40,
		MATH_POLAR2CARTESIAN = 41,
		MATH_CARTESIAN2POLAR = 42,
		MATH_WRAP = 43,
		MATH_WRAPF = 44,
		LOGIC_MAX = 45,
		LOGIC_MIN = 46,
		LOGIC_CLAMP = 47,
		LOGIC_NEAREST_PO2 = 48,
		OBJ_WEAKREF = 49,
		FUNC_FUNCREF = 50,
		TYPE_CONVERT = 51,
		TYPE_OF = 52,
		TYPE_EXISTS = 53,
		TEXT_CHAR = 54,
		TEXT_STR = 55,
		TEXT_PRINT = 56,
		TEXT_PRINTERR = 57,
		TEXT_PRINTRAW = 58,
		VAR_TO_STR = 59,
		STR_TO_VAR = 60,
		VAR_TO_BYTES = 61,
		BYTES_TO_VAR = 62,
		COLORN = 63,
		MATH_SMOOTHSTEP = 64,
		MATH_POSMOD = 65,
		MATH_LERP_ANGLE = 66,
		TEXT_ORD = 67,
		FUNC_MAX = 68,
	};

	// constants


	static VisualScriptBuiltinFunc *_new();

	// methods
	VisualScriptBuiltinFunc::BuiltinFunc get_func();
	void set_func(const int64_t which);

};

}

#endif