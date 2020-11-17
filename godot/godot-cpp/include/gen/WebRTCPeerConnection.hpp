#ifndef GODOT_CPP_WEBRTCPEERCONNECTION_HPP
#define GODOT_CPP_WEBRTCPEERCONNECTION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "WebRTCPeerConnection.hpp"

#include "Reference.hpp"
namespace godot {

class WebRTCDataChannel;

class WebRTCPeerConnection : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_add_ice_candidate;
		godot_method_bind *mb_close;
		godot_method_bind *mb_create_data_channel;
		godot_method_bind *mb_create_offer;
		godot_method_bind *mb_get_connection_state;
		godot_method_bind *mb_initialize;
		godot_method_bind *mb_poll;
		godot_method_bind *mb_set_local_description;
		godot_method_bind *mb_set_remote_description;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "WebRTCPeerConnection"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ConnectionState {
		STATE_NEW = 0,
		STATE_CONNECTING = 1,
		STATE_CONNECTED = 2,
		STATE_DISCONNECTED = 3,
		STATE_FAILED = 4,
		STATE_CLOSED = 5,
	};

	// constants


	static WebRTCPeerConnection *_new();

	// methods
	Error add_ice_candidate(const String media, const int64_t index, const String name);
	void close();
	Ref<WebRTCDataChannel> create_data_channel(const String label, const Dictionary options = {});
	Error create_offer();
	WebRTCPeerConnection::ConnectionState get_connection_state() const;
	Error initialize(const Dictionary configuration = {});
	Error poll();
	Error set_local_description(const String type, const String sdp);
	Error set_remote_description(const String type, const String sdp);

};

}

#endif