#include "Mesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Shape.hpp"
#include "Mesh.hpp"
#include "TriangleMesh.hpp"
#include "Material.hpp"


namespace godot {


Mesh::___method_bindings Mesh::___mb = {};

void Mesh::___init_method_bindings() {
	___mb.mb_create_convex_shape = godot::api->godot_method_bind_get_method("Mesh", "create_convex_shape");
	___mb.mb_create_outline = godot::api->godot_method_bind_get_method("Mesh", "create_outline");
	___mb.mb_create_trimesh_shape = godot::api->godot_method_bind_get_method("Mesh", "create_trimesh_shape");
	___mb.mb_generate_triangle_mesh = godot::api->godot_method_bind_get_method("Mesh", "generate_triangle_mesh");
	___mb.mb_get_aabb = godot::api->godot_method_bind_get_method("Mesh", "get_aabb");
	___mb.mb_get_faces = godot::api->godot_method_bind_get_method("Mesh", "get_faces");
	___mb.mb_get_lightmap_size_hint = godot::api->godot_method_bind_get_method("Mesh", "get_lightmap_size_hint");
	___mb.mb_get_surface_count = godot::api->godot_method_bind_get_method("Mesh", "get_surface_count");
	___mb.mb_set_lightmap_size_hint = godot::api->godot_method_bind_get_method("Mesh", "set_lightmap_size_hint");
	___mb.mb_surface_get_arrays = godot::api->godot_method_bind_get_method("Mesh", "surface_get_arrays");
	___mb.mb_surface_get_blend_shape_arrays = godot::api->godot_method_bind_get_method("Mesh", "surface_get_blend_shape_arrays");
	___mb.mb_surface_get_material = godot::api->godot_method_bind_get_method("Mesh", "surface_get_material");
	___mb.mb_surface_set_material = godot::api->godot_method_bind_get_method("Mesh", "surface_set_material");
}

Ref<Shape> Mesh::create_convex_shape() const {
	return Ref<Shape>::__internal_constructor(___godot_icall_Object(___mb.mb_create_convex_shape, (const Object *) this));
}

Ref<Mesh> Mesh::create_outline(const real_t margin) const {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object_float(___mb.mb_create_outline, (const Object *) this, margin));
}

Ref<Shape> Mesh::create_trimesh_shape() const {
	return Ref<Shape>::__internal_constructor(___godot_icall_Object(___mb.mb_create_trimesh_shape, (const Object *) this));
}

Ref<TriangleMesh> Mesh::generate_triangle_mesh() const {
	return Ref<TriangleMesh>::__internal_constructor(___godot_icall_Object(___mb.mb_generate_triangle_mesh, (const Object *) this));
}

AABB Mesh::get_aabb() const {
	return ___godot_icall_AABB(___mb.mb_get_aabb, (const Object *) this);
}

PoolVector3Array Mesh::get_faces() const {
	return ___godot_icall_PoolVector3Array(___mb.mb_get_faces, (const Object *) this);
}

Vector2 Mesh::get_lightmap_size_hint() const {
	return ___godot_icall_Vector2(___mb.mb_get_lightmap_size_hint, (const Object *) this);
}

int64_t Mesh::get_surface_count() const {
	return ___godot_icall_int(___mb.mb_get_surface_count, (const Object *) this);
}

void Mesh::set_lightmap_size_hint(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_lightmap_size_hint, (const Object *) this, size);
}

Array Mesh::surface_get_arrays(const int64_t surf_idx) const {
	return ___godot_icall_Array_int(___mb.mb_surface_get_arrays, (const Object *) this, surf_idx);
}

Array Mesh::surface_get_blend_shape_arrays(const int64_t surf_idx) const {
	return ___godot_icall_Array_int(___mb.mb_surface_get_blend_shape_arrays, (const Object *) this, surf_idx);
}

Ref<Material> Mesh::surface_get_material(const int64_t surf_idx) const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object_int(___mb.mb_surface_get_material, (const Object *) this, surf_idx));
}

void Mesh::surface_set_material(const int64_t surf_idx, const Ref<Material> material) {
	___godot_icall_void_int_Object(___mb.mb_surface_set_material, (const Object *) this, surf_idx, material.ptr());
}

}