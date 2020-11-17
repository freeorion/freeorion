#include "GDNative.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "GDNativeLibrary.hpp"


namespace godot {


GDNative::___method_bindings GDNative::___mb = {};

void GDNative::___init_method_bindings() {
	___mb.mb_call_native = godot::api->godot_method_bind_get_method("GDNative", "call_native");
	___mb.mb_get_library = godot::api->godot_method_bind_get_method("GDNative", "get_library");
	___mb.mb_initialize = godot::api->godot_method_bind_get_method("GDNative", "initialize");
	___mb.mb_set_library = godot::api->godot_method_bind_get_method("GDNative", "set_library");
	___mb.mb_terminate = godot::api->godot_method_bind_get_method("GDNative", "terminate");
}

GDNative *GDNative::_new()
{
	return (GDNative *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"GDNative")());
}
Variant GDNative::call_native(const String calling_type, const String procedure_name, const Array arguments) {
	return ___godot_icall_Variant_String_String_Array(___mb.mb_call_native, (const Object *) this, calling_type, procedure_name, arguments);
}

Ref<GDNativeLibrary> GDNative::get_library() const {
	return Ref<GDNativeLibrary>::__internal_constructor(___godot_icall_Object(___mb.mb_get_library, (const Object *) this));
}

bool GDNative::initialize() {
	return ___godot_icall_bool(___mb.mb_initialize, (const Object *) this);
}

void GDNative::set_library(const Ref<GDNativeLibrary> library) {
	___godot_icall_void_Object(___mb.mb_set_library, (const Object *) this, library.ptr());
}

bool GDNative::terminate() {
	return ___godot_icall_bool(___mb.mb_terminate, (const Object *) this);
}

}