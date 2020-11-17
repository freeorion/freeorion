#ifndef GODOT_CPP_PERFORMANCE_HPP
#define GODOT_CPP_PERFORMANCE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Object.hpp"
namespace godot {


class Performance : public Object {
	static Performance *_singleton;

	Performance();

	struct ___method_bindings {
		godot_method_bind *mb_get_monitor;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline Performance *get_singleton()
	{
		if (!Performance::_singleton) {
			Performance::_singleton = new Performance;
		}
		return Performance::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "Performance"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum Monitor {
		TIME_FPS = 0,
		TIME_PROCESS = 1,
		TIME_PHYSICS_PROCESS = 2,
		MEMORY_STATIC = 3,
		MEMORY_DYNAMIC = 4,
		MEMORY_STATIC_MAX = 5,
		MEMORY_DYNAMIC_MAX = 6,
		MEMORY_MESSAGE_BUFFER_MAX = 7,
		OBJECT_COUNT = 8,
		OBJECT_RESOURCE_COUNT = 9,
		OBJECT_NODE_COUNT = 10,
		OBJECT_ORPHAN_NODE_COUNT = 11,
		RENDER_OBJECTS_IN_FRAME = 12,
		RENDER_VERTICES_IN_FRAME = 13,
		RENDER_MATERIAL_CHANGES_IN_FRAME = 14,
		RENDER_SHADER_CHANGES_IN_FRAME = 15,
		RENDER_SURFACE_CHANGES_IN_FRAME = 16,
		RENDER_DRAW_CALLS_IN_FRAME = 17,
		RENDER_VIDEO_MEM_USED = 18,
		RENDER_TEXTURE_MEM_USED = 19,
		RENDER_VERTEX_MEM_USED = 20,
		RENDER_USAGE_VIDEO_MEM_TOTAL = 21,
		PHYSICS_2D_ACTIVE_OBJECTS = 22,
		PHYSICS_2D_COLLISION_PAIRS = 23,
		PHYSICS_2D_ISLAND_COUNT = 24,
		PHYSICS_3D_ACTIVE_OBJECTS = 25,
		PHYSICS_3D_COLLISION_PAIRS = 26,
		PHYSICS_3D_ISLAND_COUNT = 27,
		AUDIO_OUTPUT_LATENCY = 28,
		MONITOR_MAX = 29,
	};

	// constants

	// methods
	real_t get_monitor(const int64_t monitor) const;

};

}

#endif