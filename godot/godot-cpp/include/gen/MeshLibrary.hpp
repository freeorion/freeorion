#ifndef GODOT_CPP_MESHLIBRARY_HPP
#define GODOT_CPP_MESHLIBRARY_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class Mesh;
class NavigationMesh;
class Texture;

class MeshLibrary : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_clear;
		godot_method_bind *mb_create_item;
		godot_method_bind *mb_find_item_by_name;
		godot_method_bind *mb_get_item_list;
		godot_method_bind *mb_get_item_mesh;
		godot_method_bind *mb_get_item_name;
		godot_method_bind *mb_get_item_navmesh;
		godot_method_bind *mb_get_item_navmesh_transform;
		godot_method_bind *mb_get_item_preview;
		godot_method_bind *mb_get_item_shapes;
		godot_method_bind *mb_get_last_unused_item_id;
		godot_method_bind *mb_remove_item;
		godot_method_bind *mb_set_item_mesh;
		godot_method_bind *mb_set_item_name;
		godot_method_bind *mb_set_item_navmesh;
		godot_method_bind *mb_set_item_navmesh_transform;
		godot_method_bind *mb_set_item_preview;
		godot_method_bind *mb_set_item_shapes;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "MeshLibrary"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static MeshLibrary *_new();

	// methods
	void clear();
	void create_item(const int64_t id);
	int64_t find_item_by_name(const String name) const;
	PoolIntArray get_item_list() const;
	Ref<Mesh> get_item_mesh(const int64_t id) const;
	String get_item_name(const int64_t id) const;
	Ref<NavigationMesh> get_item_navmesh(const int64_t id) const;
	Transform get_item_navmesh_transform(const int64_t id) const;
	Ref<Texture> get_item_preview(const int64_t id) const;
	Array get_item_shapes(const int64_t id) const;
	int64_t get_last_unused_item_id() const;
	void remove_item(const int64_t id);
	void set_item_mesh(const int64_t id, const Ref<Mesh> mesh);
	void set_item_name(const int64_t id, const String name);
	void set_item_navmesh(const int64_t id, const Ref<NavigationMesh> navmesh);
	void set_item_navmesh_transform(const int64_t id, const Transform navmesh);
	void set_item_preview(const int64_t id, const Ref<Texture> texture);
	void set_item_shapes(const int64_t id, const Array shapes);

};

}

#endif