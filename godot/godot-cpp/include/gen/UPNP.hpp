#ifndef GODOT_CPP_UPNP_HPP
#define GODOT_CPP_UPNP_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Reference.hpp"
namespace godot {

class UPNPDevice;

class UPNP : public Reference {
	struct ___method_bindings {
		godot_method_bind *mb_add_device;
		godot_method_bind *mb_add_port_mapping;
		godot_method_bind *mb_clear_devices;
		godot_method_bind *mb_delete_port_mapping;
		godot_method_bind *mb_discover;
		godot_method_bind *mb_get_device;
		godot_method_bind *mb_get_device_count;
		godot_method_bind *mb_get_discover_local_port;
		godot_method_bind *mb_get_discover_multicast_if;
		godot_method_bind *mb_get_gateway;
		godot_method_bind *mb_is_discover_ipv6;
		godot_method_bind *mb_query_external_address;
		godot_method_bind *mb_remove_device;
		godot_method_bind *mb_set_device;
		godot_method_bind *mb_set_discover_ipv6;
		godot_method_bind *mb_set_discover_local_port;
		godot_method_bind *mb_set_discover_multicast_if;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "UPNP"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum UPNPResult {
		UPNP_RESULT_SUCCESS = 0,
		UPNP_RESULT_NOT_AUTHORIZED = 1,
		UPNP_RESULT_PORT_MAPPING_NOT_FOUND = 2,
		UPNP_RESULT_INCONSISTENT_PARAMETERS = 3,
		UPNP_RESULT_NO_SUCH_ENTRY_IN_ARRAY = 4,
		UPNP_RESULT_ACTION_FAILED = 5,
		UPNP_RESULT_SRC_IP_WILDCARD_NOT_PERMITTED = 6,
		UPNP_RESULT_EXT_PORT_WILDCARD_NOT_PERMITTED = 7,
		UPNP_RESULT_INT_PORT_WILDCARD_NOT_PERMITTED = 8,
		UPNP_RESULT_REMOTE_HOST_MUST_BE_WILDCARD = 9,
		UPNP_RESULT_EXT_PORT_MUST_BE_WILDCARD = 10,
		UPNP_RESULT_NO_PORT_MAPS_AVAILABLE = 11,
		UPNP_RESULT_CONFLICT_WITH_OTHER_MECHANISM = 12,
		UPNP_RESULT_CONFLICT_WITH_OTHER_MAPPING = 13,
		UPNP_RESULT_SAME_PORT_VALUES_REQUIRED = 14,
		UPNP_RESULT_ONLY_PERMANENT_LEASE_SUPPORTED = 15,
		UPNP_RESULT_INVALID_GATEWAY = 16,
		UPNP_RESULT_INVALID_PORT = 17,
		UPNP_RESULT_INVALID_PROTOCOL = 18,
		UPNP_RESULT_INVALID_DURATION = 19,
		UPNP_RESULT_INVALID_ARGS = 20,
		UPNP_RESULT_INVALID_RESPONSE = 21,
		UPNP_RESULT_INVALID_PARAM = 22,
		UPNP_RESULT_HTTP_ERROR = 23,
		UPNP_RESULT_SOCKET_ERROR = 24,
		UPNP_RESULT_MEM_ALLOC_ERROR = 25,
		UPNP_RESULT_NO_GATEWAY = 26,
		UPNP_RESULT_NO_DEVICES = 27,
		UPNP_RESULT_UNKNOWN_ERROR = 28,
	};

	// constants


	static UPNP *_new();

	// methods
	void add_device(const Ref<UPNPDevice> device);
	int64_t add_port_mapping(const int64_t port, const int64_t port_internal = 0, const String desc = "", const String proto = "UDP", const int64_t duration = 0) const;
	void clear_devices();
	int64_t delete_port_mapping(const int64_t port, const String proto = "UDP") const;
	int64_t discover(const int64_t timeout = 2000, const int64_t ttl = 2, const String device_filter = "InternetGatewayDevice");
	Ref<UPNPDevice> get_device(const int64_t index) const;
	int64_t get_device_count() const;
	int64_t get_discover_local_port() const;
	String get_discover_multicast_if() const;
	Ref<UPNPDevice> get_gateway() const;
	bool is_discover_ipv6() const;
	String query_external_address() const;
	void remove_device(const int64_t index);
	void set_device(const int64_t index, const Ref<UPNPDevice> device);
	void set_discover_ipv6(const bool ipv6);
	void set_discover_local_port(const int64_t port);
	void set_discover_multicast_if(const String m_if);

};

}

#endif