#ifndef GODOT_CPP_WEBRTCMULTIPLAYER_HPP
#define GODOT_CPP_WEBRTCMULTIPLAYER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "NetworkedMultiplayerPeer.hpp"
namespace godot {

class WebRTCPeerConnection;

class WebRTCMultiplayer : public NetworkedMultiplayerPeer {
	struct ___method_bindings {
		godot_method_bind *mb_add_peer;
		godot_method_bind *mb_close;
		godot_method_bind *mb_get_peer;
		godot_method_bind *mb_get_peers;
		godot_method_bind *mb_has_peer;
		godot_method_bind *mb_initialize;
		godot_method_bind *mb_remove_peer;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "WebRTCMultiplayer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static WebRTCMultiplayer *_new();

	// methods
	Error add_peer(const Ref<WebRTCPeerConnection> peer, const int64_t peer_id, const int64_t unreliable_lifetime = 1);
	void close();
	Dictionary get_peer(const int64_t peer_id);
	Dictionary get_peers();
	bool has_peer(const int64_t peer_id);
	Error initialize(const int64_t peer_id, const bool server_compatibility = false);
	void remove_peer(const int64_t peer_id);

};

}

#endif