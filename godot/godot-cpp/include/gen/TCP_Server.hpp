#ifndef GODOT_CPP_TCP_SERVER_HPP
#define GODOT_CPP_TCP_SERVER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class StreamPeerTCP;

class TCP_Server : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_is_connection_available;
		godot_method_bind *mb_is_listening;
		godot_method_bind *mb_listen;
		godot_method_bind *mb_stop;
		godot_method_bind *mb_take_connection;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "TCP_Server"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static TCP_Server *_new();

	// methods
	bool is_connection_available() const;
	bool is_listening() const;
	Error listen(const int64_t port, const String bind_address = "*");
	void stop();
	Ref<StreamPeerTCP> take_connection();

};

}

#endif