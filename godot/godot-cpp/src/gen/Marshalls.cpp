#include "Marshalls.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Marshalls *Marshalls::_singleton = NULL;


Marshalls::Marshalls() {
	_owner = godot::api->godot_global_get_singleton((char *) "Marshalls");
}


Marshalls::___method_bindings Marshalls::___mb = {};

void Marshalls::___init_method_bindings() {
	___mb.mb_base64_to_raw = godot::api->godot_method_bind_get_method("_Marshalls", "base64_to_raw");
	___mb.mb_base64_to_utf8 = godot::api->godot_method_bind_get_method("_Marshalls", "base64_to_utf8");
	___mb.mb_base64_to_variant = godot::api->godot_method_bind_get_method("_Marshalls", "base64_to_variant");
	___mb.mb_raw_to_base64 = godot::api->godot_method_bind_get_method("_Marshalls", "raw_to_base64");
	___mb.mb_utf8_to_base64 = godot::api->godot_method_bind_get_method("_Marshalls", "utf8_to_base64");
	___mb.mb_variant_to_base64 = godot::api->godot_method_bind_get_method("_Marshalls", "variant_to_base64");
}

PoolByteArray Marshalls::base64_to_raw(const String base64_str) {
	return ___godot_icall_PoolByteArray_String(___mb.mb_base64_to_raw, (const Object *) this, base64_str);
}

String Marshalls::base64_to_utf8(const String base64_str) {
	return ___godot_icall_String_String(___mb.mb_base64_to_utf8, (const Object *) this, base64_str);
}

Variant Marshalls::base64_to_variant(const String base64_str, const bool allow_objects) {
	return ___godot_icall_Variant_String_bool(___mb.mb_base64_to_variant, (const Object *) this, base64_str, allow_objects);
}

String Marshalls::raw_to_base64(const PoolByteArray array) {
	return ___godot_icall_String_PoolByteArray(___mb.mb_raw_to_base64, (const Object *) this, array);
}

String Marshalls::utf8_to_base64(const String utf8_str) {
	return ___godot_icall_String_String(___mb.mb_utf8_to_base64, (const Object *) this, utf8_str);
}

String Marshalls::variant_to_base64(const Variant variant, const bool full_objects) {
	return ___godot_icall_String_Variant_bool(___mb.mb_variant_to_base64, (const Object *) this, variant, full_objects);
}

}