#ifndef GODOT_CPP_WEBRTCPEERCONNECTIONGDNATIVE_HPP
#define GODOT_CPP_WEBRTCPEERCONNECTIONGDNATIVE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "WebRTCPeerConnection.hpp"
namespace godot {


class WebRTCPeerConnectionGDNative : public WebRTCPeerConnection {
	struct ___method_bindings {
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "WebRTCPeerConnectionGDNative"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static WebRTCPeerConnectionGDNative *_new();

	// methods

};

}

#endif