#include "Crypto.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "CryptoKey.hpp"
#include "X509Certificate.hpp"


namespace godot {


Crypto::___method_bindings Crypto::___mb = {};

void Crypto::___init_method_bindings() {
	___mb.mb_generate_random_bytes = godot::api->godot_method_bind_get_method("Crypto", "generate_random_bytes");
	___mb.mb_generate_rsa = godot::api->godot_method_bind_get_method("Crypto", "generate_rsa");
	___mb.mb_generate_self_signed_certificate = godot::api->godot_method_bind_get_method("Crypto", "generate_self_signed_certificate");
}

Crypto *Crypto::_new()
{
	return (Crypto *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Crypto")());
}
PoolByteArray Crypto::generate_random_bytes(const int64_t size) {
	return ___godot_icall_PoolByteArray_int(___mb.mb_generate_random_bytes, (const Object *) this, size);
}

Ref<CryptoKey> Crypto::generate_rsa(const int64_t size) {
	return Ref<CryptoKey>::__internal_constructor(___godot_icall_Object_int(___mb.mb_generate_rsa, (const Object *) this, size));
}

Ref<X509Certificate> Crypto::generate_self_signed_certificate(const Ref<CryptoKey> key, const String issuer_name, const String not_before, const String not_after) {
	return Ref<X509Certificate>::__internal_constructor(___godot_icall_Object_Object_String_String_String(___mb.mb_generate_self_signed_certificate, (const Object *) this, key.ptr(), issuer_name, not_before, not_after));
}

}