#ifndef GODOT_CPP_CRYPTO_HPP
#define GODOT_CPP_CRYPTO_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class CryptoKey;
class X509Certificate;

class Crypto : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_generate_random_bytes;
		godot_method_bind *mb_generate_rsa;
		godot_method_bind *mb_generate_self_signed_certificate;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Crypto"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static Crypto *_new();

	// methods
	PoolByteArray generate_random_bytes(const int64_t size);
	Ref<CryptoKey> generate_rsa(const int64_t size);
	Ref<X509Certificate> generate_self_signed_certificate(const Ref<CryptoKey> key, const String issuer_name = "CN=myserver,O=myorganisation,C=IT", const String not_before = "20140101000000", const String not_after = "20340101000000");

};

}

#endif