#include "VisualServer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"
#include "Image.hpp"


namespace godot {


VisualServer *VisualServer::_singleton = NULL;


VisualServer::VisualServer() {
	_owner = godot::api->godot_global_get_singleton((char *) "VisualServer");
}


VisualServer::___method_bindings VisualServer::___mb = {};

void VisualServer::___init_method_bindings() {
	___mb.mb_black_bars_set_images = godot::api->godot_method_bind_get_method("VisualServer", "black_bars_set_images");
	___mb.mb_black_bars_set_margins = godot::api->godot_method_bind_get_method("VisualServer", "black_bars_set_margins");
	___mb.mb_camera_create = godot::api->godot_method_bind_get_method("VisualServer", "camera_create");
	___mb.mb_camera_set_cull_mask = godot::api->godot_method_bind_get_method("VisualServer", "camera_set_cull_mask");
	___mb.mb_camera_set_environment = godot::api->godot_method_bind_get_method("VisualServer", "camera_set_environment");
	___mb.mb_camera_set_frustum = godot::api->godot_method_bind_get_method("VisualServer", "camera_set_frustum");
	___mb.mb_camera_set_orthogonal = godot::api->godot_method_bind_get_method("VisualServer", "camera_set_orthogonal");
	___mb.mb_camera_set_perspective = godot::api->godot_method_bind_get_method("VisualServer", "camera_set_perspective");
	___mb.mb_camera_set_transform = godot::api->godot_method_bind_get_method("VisualServer", "camera_set_transform");
	___mb.mb_camera_set_use_vertical_aspect = godot::api->godot_method_bind_get_method("VisualServer", "camera_set_use_vertical_aspect");
	___mb.mb_canvas_create = godot::api->godot_method_bind_get_method("VisualServer", "canvas_create");
	___mb.mb_canvas_item_add_circle = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_circle");
	___mb.mb_canvas_item_add_clip_ignore = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_clip_ignore");
	___mb.mb_canvas_item_add_line = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_line");
	___mb.mb_canvas_item_add_mesh = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_mesh");
	___mb.mb_canvas_item_add_multimesh = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_multimesh");
	___mb.mb_canvas_item_add_nine_patch = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_nine_patch");
	___mb.mb_canvas_item_add_particles = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_particles");
	___mb.mb_canvas_item_add_polygon = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_polygon");
	___mb.mb_canvas_item_add_polyline = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_polyline");
	___mb.mb_canvas_item_add_primitive = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_primitive");
	___mb.mb_canvas_item_add_rect = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_rect");
	___mb.mb_canvas_item_add_set_transform = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_set_transform");
	___mb.mb_canvas_item_add_texture_rect = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_texture_rect");
	___mb.mb_canvas_item_add_texture_rect_region = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_texture_rect_region");
	___mb.mb_canvas_item_add_triangle_array = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_add_triangle_array");
	___mb.mb_canvas_item_clear = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_clear");
	___mb.mb_canvas_item_create = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_create");
	___mb.mb_canvas_item_set_clip = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_clip");
	___mb.mb_canvas_item_set_copy_to_backbuffer = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_copy_to_backbuffer");
	___mb.mb_canvas_item_set_custom_rect = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_custom_rect");
	___mb.mb_canvas_item_set_distance_field_mode = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_distance_field_mode");
	___mb.mb_canvas_item_set_draw_behind_parent = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_draw_behind_parent");
	___mb.mb_canvas_item_set_draw_index = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_draw_index");
	___mb.mb_canvas_item_set_light_mask = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_light_mask");
	___mb.mb_canvas_item_set_material = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_material");
	___mb.mb_canvas_item_set_modulate = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_modulate");
	___mb.mb_canvas_item_set_parent = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_parent");
	___mb.mb_canvas_item_set_self_modulate = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_self_modulate");
	___mb.mb_canvas_item_set_sort_children_by_y = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_sort_children_by_y");
	___mb.mb_canvas_item_set_transform = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_transform");
	___mb.mb_canvas_item_set_use_parent_material = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_use_parent_material");
	___mb.mb_canvas_item_set_visible = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_visible");
	___mb.mb_canvas_item_set_z_as_relative_to_parent = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_z_as_relative_to_parent");
	___mb.mb_canvas_item_set_z_index = godot::api->godot_method_bind_get_method("VisualServer", "canvas_item_set_z_index");
	___mb.mb_canvas_light_attach_to_canvas = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_attach_to_canvas");
	___mb.mb_canvas_light_create = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_create");
	___mb.mb_canvas_light_occluder_attach_to_canvas = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_occluder_attach_to_canvas");
	___mb.mb_canvas_light_occluder_create = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_occluder_create");
	___mb.mb_canvas_light_occluder_set_enabled = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_occluder_set_enabled");
	___mb.mb_canvas_light_occluder_set_light_mask = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_occluder_set_light_mask");
	___mb.mb_canvas_light_occluder_set_polygon = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_occluder_set_polygon");
	___mb.mb_canvas_light_occluder_set_transform = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_occluder_set_transform");
	___mb.mb_canvas_light_set_color = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_color");
	___mb.mb_canvas_light_set_enabled = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_enabled");
	___mb.mb_canvas_light_set_energy = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_energy");
	___mb.mb_canvas_light_set_height = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_height");
	___mb.mb_canvas_light_set_item_cull_mask = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_item_cull_mask");
	___mb.mb_canvas_light_set_item_shadow_cull_mask = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_item_shadow_cull_mask");
	___mb.mb_canvas_light_set_layer_range = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_layer_range");
	___mb.mb_canvas_light_set_mode = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_mode");
	___mb.mb_canvas_light_set_scale = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_scale");
	___mb.mb_canvas_light_set_shadow_buffer_size = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_shadow_buffer_size");
	___mb.mb_canvas_light_set_shadow_color = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_shadow_color");
	___mb.mb_canvas_light_set_shadow_enabled = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_shadow_enabled");
	___mb.mb_canvas_light_set_shadow_filter = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_shadow_filter");
	___mb.mb_canvas_light_set_shadow_gradient_length = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_shadow_gradient_length");
	___mb.mb_canvas_light_set_shadow_smooth = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_shadow_smooth");
	___mb.mb_canvas_light_set_texture = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_texture");
	___mb.mb_canvas_light_set_texture_offset = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_texture_offset");
	___mb.mb_canvas_light_set_transform = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_transform");
	___mb.mb_canvas_light_set_z_range = godot::api->godot_method_bind_get_method("VisualServer", "canvas_light_set_z_range");
	___mb.mb_canvas_occluder_polygon_create = godot::api->godot_method_bind_get_method("VisualServer", "canvas_occluder_polygon_create");
	___mb.mb_canvas_occluder_polygon_set_cull_mode = godot::api->godot_method_bind_get_method("VisualServer", "canvas_occluder_polygon_set_cull_mode");
	___mb.mb_canvas_occluder_polygon_set_shape = godot::api->godot_method_bind_get_method("VisualServer", "canvas_occluder_polygon_set_shape");
	___mb.mb_canvas_occluder_polygon_set_shape_as_lines = godot::api->godot_method_bind_get_method("VisualServer", "canvas_occluder_polygon_set_shape_as_lines");
	___mb.mb_canvas_set_item_mirroring = godot::api->godot_method_bind_get_method("VisualServer", "canvas_set_item_mirroring");
	___mb.mb_canvas_set_modulate = godot::api->godot_method_bind_get_method("VisualServer", "canvas_set_modulate");
	___mb.mb_directional_light_create = godot::api->godot_method_bind_get_method("VisualServer", "directional_light_create");
	___mb.mb_draw = godot::api->godot_method_bind_get_method("VisualServer", "draw");
	___mb.mb_environment_create = godot::api->godot_method_bind_get_method("VisualServer", "environment_create");
	___mb.mb_environment_set_adjustment = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_adjustment");
	___mb.mb_environment_set_ambient_light = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_ambient_light");
	___mb.mb_environment_set_background = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_background");
	___mb.mb_environment_set_bg_color = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_bg_color");
	___mb.mb_environment_set_bg_energy = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_bg_energy");
	___mb.mb_environment_set_canvas_max_layer = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_canvas_max_layer");
	___mb.mb_environment_set_dof_blur_far = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_dof_blur_far");
	___mb.mb_environment_set_dof_blur_near = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_dof_blur_near");
	___mb.mb_environment_set_fog = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_fog");
	___mb.mb_environment_set_fog_depth = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_fog_depth");
	___mb.mb_environment_set_fog_height = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_fog_height");
	___mb.mb_environment_set_glow = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_glow");
	___mb.mb_environment_set_sky = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_sky");
	___mb.mb_environment_set_sky_custom_fov = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_sky_custom_fov");
	___mb.mb_environment_set_sky_orientation = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_sky_orientation");
	___mb.mb_environment_set_ssao = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_ssao");
	___mb.mb_environment_set_ssr = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_ssr");
	___mb.mb_environment_set_tonemap = godot::api->godot_method_bind_get_method("VisualServer", "environment_set_tonemap");
	___mb.mb_finish = godot::api->godot_method_bind_get_method("VisualServer", "finish");
	___mb.mb_force_draw = godot::api->godot_method_bind_get_method("VisualServer", "force_draw");
	___mb.mb_force_sync = godot::api->godot_method_bind_get_method("VisualServer", "force_sync");
	___mb.mb_free_rid = godot::api->godot_method_bind_get_method("VisualServer", "free_rid");
	___mb.mb_get_render_info = godot::api->godot_method_bind_get_method("VisualServer", "get_render_info");
	___mb.mb_get_test_cube = godot::api->godot_method_bind_get_method("VisualServer", "get_test_cube");
	___mb.mb_get_test_texture = godot::api->godot_method_bind_get_method("VisualServer", "get_test_texture");
	___mb.mb_get_video_adapter_name = godot::api->godot_method_bind_get_method("VisualServer", "get_video_adapter_name");
	___mb.mb_get_video_adapter_vendor = godot::api->godot_method_bind_get_method("VisualServer", "get_video_adapter_vendor");
	___mb.mb_get_white_texture = godot::api->godot_method_bind_get_method("VisualServer", "get_white_texture");
	___mb.mb_gi_probe_create = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_create");
	___mb.mb_gi_probe_get_bias = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_get_bias");
	___mb.mb_gi_probe_get_bounds = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_get_bounds");
	___mb.mb_gi_probe_get_cell_size = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_get_cell_size");
	___mb.mb_gi_probe_get_dynamic_data = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_get_dynamic_data");
	___mb.mb_gi_probe_get_dynamic_range = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_get_dynamic_range");
	___mb.mb_gi_probe_get_energy = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_get_energy");
	___mb.mb_gi_probe_get_normal_bias = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_get_normal_bias");
	___mb.mb_gi_probe_get_propagation = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_get_propagation");
	___mb.mb_gi_probe_get_to_cell_xform = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_get_to_cell_xform");
	___mb.mb_gi_probe_is_compressed = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_is_compressed");
	___mb.mb_gi_probe_is_interior = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_is_interior");
	___mb.mb_gi_probe_set_bias = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_bias");
	___mb.mb_gi_probe_set_bounds = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_bounds");
	___mb.mb_gi_probe_set_cell_size = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_cell_size");
	___mb.mb_gi_probe_set_compress = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_compress");
	___mb.mb_gi_probe_set_dynamic_data = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_dynamic_data");
	___mb.mb_gi_probe_set_dynamic_range = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_dynamic_range");
	___mb.mb_gi_probe_set_energy = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_energy");
	___mb.mb_gi_probe_set_interior = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_interior");
	___mb.mb_gi_probe_set_normal_bias = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_normal_bias");
	___mb.mb_gi_probe_set_propagation = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_propagation");
	___mb.mb_gi_probe_set_to_cell_xform = godot::api->godot_method_bind_get_method("VisualServer", "gi_probe_set_to_cell_xform");
	___mb.mb_has_changed = godot::api->godot_method_bind_get_method("VisualServer", "has_changed");
	___mb.mb_has_feature = godot::api->godot_method_bind_get_method("VisualServer", "has_feature");
	___mb.mb_has_os_feature = godot::api->godot_method_bind_get_method("VisualServer", "has_os_feature");
	___mb.mb_immediate_begin = godot::api->godot_method_bind_get_method("VisualServer", "immediate_begin");
	___mb.mb_immediate_clear = godot::api->godot_method_bind_get_method("VisualServer", "immediate_clear");
	___mb.mb_immediate_color = godot::api->godot_method_bind_get_method("VisualServer", "immediate_color");
	___mb.mb_immediate_create = godot::api->godot_method_bind_get_method("VisualServer", "immediate_create");
	___mb.mb_immediate_end = godot::api->godot_method_bind_get_method("VisualServer", "immediate_end");
	___mb.mb_immediate_get_material = godot::api->godot_method_bind_get_method("VisualServer", "immediate_get_material");
	___mb.mb_immediate_normal = godot::api->godot_method_bind_get_method("VisualServer", "immediate_normal");
	___mb.mb_immediate_set_material = godot::api->godot_method_bind_get_method("VisualServer", "immediate_set_material");
	___mb.mb_immediate_tangent = godot::api->godot_method_bind_get_method("VisualServer", "immediate_tangent");
	___mb.mb_immediate_uv = godot::api->godot_method_bind_get_method("VisualServer", "immediate_uv");
	___mb.mb_immediate_uv2 = godot::api->godot_method_bind_get_method("VisualServer", "immediate_uv2");
	___mb.mb_immediate_vertex = godot::api->godot_method_bind_get_method("VisualServer", "immediate_vertex");
	___mb.mb_immediate_vertex_2d = godot::api->godot_method_bind_get_method("VisualServer", "immediate_vertex_2d");
	___mb.mb_init = godot::api->godot_method_bind_get_method("VisualServer", "init");
	___mb.mb_instance_attach_object_instance_id = godot::api->godot_method_bind_get_method("VisualServer", "instance_attach_object_instance_id");
	___mb.mb_instance_attach_skeleton = godot::api->godot_method_bind_get_method("VisualServer", "instance_attach_skeleton");
	___mb.mb_instance_create = godot::api->godot_method_bind_get_method("VisualServer", "instance_create");
	___mb.mb_instance_create2 = godot::api->godot_method_bind_get_method("VisualServer", "instance_create2");
	___mb.mb_instance_geometry_set_as_instance_lod = godot::api->godot_method_bind_get_method("VisualServer", "instance_geometry_set_as_instance_lod");
	___mb.mb_instance_geometry_set_cast_shadows_setting = godot::api->godot_method_bind_get_method("VisualServer", "instance_geometry_set_cast_shadows_setting");
	___mb.mb_instance_geometry_set_draw_range = godot::api->godot_method_bind_get_method("VisualServer", "instance_geometry_set_draw_range");
	___mb.mb_instance_geometry_set_flag = godot::api->godot_method_bind_get_method("VisualServer", "instance_geometry_set_flag");
	___mb.mb_instance_geometry_set_material_override = godot::api->godot_method_bind_get_method("VisualServer", "instance_geometry_set_material_override");
	___mb.mb_instance_set_base = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_base");
	___mb.mb_instance_set_blend_shape_weight = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_blend_shape_weight");
	___mb.mb_instance_set_custom_aabb = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_custom_aabb");
	___mb.mb_instance_set_exterior = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_exterior");
	___mb.mb_instance_set_extra_visibility_margin = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_extra_visibility_margin");
	___mb.mb_instance_set_layer_mask = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_layer_mask");
	___mb.mb_instance_set_scenario = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_scenario");
	___mb.mb_instance_set_surface_material = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_surface_material");
	___mb.mb_instance_set_transform = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_transform");
	___mb.mb_instance_set_use_lightmap = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_use_lightmap");
	___mb.mb_instance_set_visible = godot::api->godot_method_bind_get_method("VisualServer", "instance_set_visible");
	___mb.mb_instances_cull_aabb = godot::api->godot_method_bind_get_method("VisualServer", "instances_cull_aabb");
	___mb.mb_instances_cull_convex = godot::api->godot_method_bind_get_method("VisualServer", "instances_cull_convex");
	___mb.mb_instances_cull_ray = godot::api->godot_method_bind_get_method("VisualServer", "instances_cull_ray");
	___mb.mb_light_directional_set_blend_splits = godot::api->godot_method_bind_get_method("VisualServer", "light_directional_set_blend_splits");
	___mb.mb_light_directional_set_shadow_depth_range_mode = godot::api->godot_method_bind_get_method("VisualServer", "light_directional_set_shadow_depth_range_mode");
	___mb.mb_light_directional_set_shadow_mode = godot::api->godot_method_bind_get_method("VisualServer", "light_directional_set_shadow_mode");
	___mb.mb_light_omni_set_shadow_detail = godot::api->godot_method_bind_get_method("VisualServer", "light_omni_set_shadow_detail");
	___mb.mb_light_omni_set_shadow_mode = godot::api->godot_method_bind_get_method("VisualServer", "light_omni_set_shadow_mode");
	___mb.mb_light_set_color = godot::api->godot_method_bind_get_method("VisualServer", "light_set_color");
	___mb.mb_light_set_cull_mask = godot::api->godot_method_bind_get_method("VisualServer", "light_set_cull_mask");
	___mb.mb_light_set_negative = godot::api->godot_method_bind_get_method("VisualServer", "light_set_negative");
	___mb.mb_light_set_param = godot::api->godot_method_bind_get_method("VisualServer", "light_set_param");
	___mb.mb_light_set_projector = godot::api->godot_method_bind_get_method("VisualServer", "light_set_projector");
	___mb.mb_light_set_reverse_cull_face_mode = godot::api->godot_method_bind_get_method("VisualServer", "light_set_reverse_cull_face_mode");
	___mb.mb_light_set_shadow = godot::api->godot_method_bind_get_method("VisualServer", "light_set_shadow");
	___mb.mb_light_set_shadow_color = godot::api->godot_method_bind_get_method("VisualServer", "light_set_shadow_color");
	___mb.mb_light_set_use_gi = godot::api->godot_method_bind_get_method("VisualServer", "light_set_use_gi");
	___mb.mb_lightmap_capture_create = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_create");
	___mb.mb_lightmap_capture_get_bounds = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_get_bounds");
	___mb.mb_lightmap_capture_get_energy = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_get_energy");
	___mb.mb_lightmap_capture_get_octree = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_get_octree");
	___mb.mb_lightmap_capture_get_octree_cell_subdiv = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_get_octree_cell_subdiv");
	___mb.mb_lightmap_capture_get_octree_cell_transform = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_get_octree_cell_transform");
	___mb.mb_lightmap_capture_set_bounds = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_set_bounds");
	___mb.mb_lightmap_capture_set_energy = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_set_energy");
	___mb.mb_lightmap_capture_set_octree = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_set_octree");
	___mb.mb_lightmap_capture_set_octree_cell_subdiv = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_set_octree_cell_subdiv");
	___mb.mb_lightmap_capture_set_octree_cell_transform = godot::api->godot_method_bind_get_method("VisualServer", "lightmap_capture_set_octree_cell_transform");
	___mb.mb_make_sphere_mesh = godot::api->godot_method_bind_get_method("VisualServer", "make_sphere_mesh");
	___mb.mb_material_create = godot::api->godot_method_bind_get_method("VisualServer", "material_create");
	___mb.mb_material_get_param = godot::api->godot_method_bind_get_method("VisualServer", "material_get_param");
	___mb.mb_material_get_param_default = godot::api->godot_method_bind_get_method("VisualServer", "material_get_param_default");
	___mb.mb_material_get_shader = godot::api->godot_method_bind_get_method("VisualServer", "material_get_shader");
	___mb.mb_material_set_line_width = godot::api->godot_method_bind_get_method("VisualServer", "material_set_line_width");
	___mb.mb_material_set_next_pass = godot::api->godot_method_bind_get_method("VisualServer", "material_set_next_pass");
	___mb.mb_material_set_param = godot::api->godot_method_bind_get_method("VisualServer", "material_set_param");
	___mb.mb_material_set_render_priority = godot::api->godot_method_bind_get_method("VisualServer", "material_set_render_priority");
	___mb.mb_material_set_shader = godot::api->godot_method_bind_get_method("VisualServer", "material_set_shader");
	___mb.mb_mesh_add_surface_from_arrays = godot::api->godot_method_bind_get_method("VisualServer", "mesh_add_surface_from_arrays");
	___mb.mb_mesh_clear = godot::api->godot_method_bind_get_method("VisualServer", "mesh_clear");
	___mb.mb_mesh_create = godot::api->godot_method_bind_get_method("VisualServer", "mesh_create");
	___mb.mb_mesh_get_blend_shape_count = godot::api->godot_method_bind_get_method("VisualServer", "mesh_get_blend_shape_count");
	___mb.mb_mesh_get_blend_shape_mode = godot::api->godot_method_bind_get_method("VisualServer", "mesh_get_blend_shape_mode");
	___mb.mb_mesh_get_custom_aabb = godot::api->godot_method_bind_get_method("VisualServer", "mesh_get_custom_aabb");
	___mb.mb_mesh_get_surface_count = godot::api->godot_method_bind_get_method("VisualServer", "mesh_get_surface_count");
	___mb.mb_mesh_remove_surface = godot::api->godot_method_bind_get_method("VisualServer", "mesh_remove_surface");
	___mb.mb_mesh_set_blend_shape_count = godot::api->godot_method_bind_get_method("VisualServer", "mesh_set_blend_shape_count");
	___mb.mb_mesh_set_blend_shape_mode = godot::api->godot_method_bind_get_method("VisualServer", "mesh_set_blend_shape_mode");
	___mb.mb_mesh_set_custom_aabb = godot::api->godot_method_bind_get_method("VisualServer", "mesh_set_custom_aabb");
	___mb.mb_mesh_surface_get_aabb = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_aabb");
	___mb.mb_mesh_surface_get_array = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_array");
	___mb.mb_mesh_surface_get_array_index_len = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_array_index_len");
	___mb.mb_mesh_surface_get_array_len = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_array_len");
	___mb.mb_mesh_surface_get_arrays = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_arrays");
	___mb.mb_mesh_surface_get_blend_shape_arrays = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_blend_shape_arrays");
	___mb.mb_mesh_surface_get_format = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_format");
	___mb.mb_mesh_surface_get_format_offset = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_format_offset");
	___mb.mb_mesh_surface_get_format_stride = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_format_stride");
	___mb.mb_mesh_surface_get_index_array = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_index_array");
	___mb.mb_mesh_surface_get_material = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_material");
	___mb.mb_mesh_surface_get_primitive_type = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_primitive_type");
	___mb.mb_mesh_surface_get_skeleton_aabb = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_get_skeleton_aabb");
	___mb.mb_mesh_surface_set_material = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_set_material");
	___mb.mb_mesh_surface_update_region = godot::api->godot_method_bind_get_method("VisualServer", "mesh_surface_update_region");
	___mb.mb_multimesh_allocate = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_allocate");
	___mb.mb_multimesh_create = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_create");
	___mb.mb_multimesh_get_aabb = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_get_aabb");
	___mb.mb_multimesh_get_instance_count = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_get_instance_count");
	___mb.mb_multimesh_get_mesh = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_get_mesh");
	___mb.mb_multimesh_get_visible_instances = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_get_visible_instances");
	___mb.mb_multimesh_instance_get_color = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_instance_get_color");
	___mb.mb_multimesh_instance_get_custom_data = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_instance_get_custom_data");
	___mb.mb_multimesh_instance_get_transform = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_instance_get_transform");
	___mb.mb_multimesh_instance_get_transform_2d = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_instance_get_transform_2d");
	___mb.mb_multimesh_instance_set_color = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_instance_set_color");
	___mb.mb_multimesh_instance_set_custom_data = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_instance_set_custom_data");
	___mb.mb_multimesh_instance_set_transform = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_instance_set_transform");
	___mb.mb_multimesh_instance_set_transform_2d = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_instance_set_transform_2d");
	___mb.mb_multimesh_set_as_bulk_array = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_set_as_bulk_array");
	___mb.mb_multimesh_set_mesh = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_set_mesh");
	___mb.mb_multimesh_set_visible_instances = godot::api->godot_method_bind_get_method("VisualServer", "multimesh_set_visible_instances");
	___mb.mb_omni_light_create = godot::api->godot_method_bind_get_method("VisualServer", "omni_light_create");
	___mb.mb_particles_create = godot::api->godot_method_bind_get_method("VisualServer", "particles_create");
	___mb.mb_particles_get_current_aabb = godot::api->godot_method_bind_get_method("VisualServer", "particles_get_current_aabb");
	___mb.mb_particles_get_emitting = godot::api->godot_method_bind_get_method("VisualServer", "particles_get_emitting");
	___mb.mb_particles_is_inactive = godot::api->godot_method_bind_get_method("VisualServer", "particles_is_inactive");
	___mb.mb_particles_request_process = godot::api->godot_method_bind_get_method("VisualServer", "particles_request_process");
	___mb.mb_particles_restart = godot::api->godot_method_bind_get_method("VisualServer", "particles_restart");
	___mb.mb_particles_set_amount = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_amount");
	___mb.mb_particles_set_custom_aabb = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_custom_aabb");
	___mb.mb_particles_set_draw_order = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_draw_order");
	___mb.mb_particles_set_draw_pass_mesh = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_draw_pass_mesh");
	___mb.mb_particles_set_draw_passes = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_draw_passes");
	___mb.mb_particles_set_emission_transform = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_emission_transform");
	___mb.mb_particles_set_emitting = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_emitting");
	___mb.mb_particles_set_explosiveness_ratio = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_explosiveness_ratio");
	___mb.mb_particles_set_fixed_fps = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_fixed_fps");
	___mb.mb_particles_set_fractional_delta = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_fractional_delta");
	___mb.mb_particles_set_lifetime = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_lifetime");
	___mb.mb_particles_set_one_shot = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_one_shot");
	___mb.mb_particles_set_pre_process_time = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_pre_process_time");
	___mb.mb_particles_set_process_material = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_process_material");
	___mb.mb_particles_set_randomness_ratio = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_randomness_ratio");
	___mb.mb_particles_set_speed_scale = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_speed_scale");
	___mb.mb_particles_set_use_local_coordinates = godot::api->godot_method_bind_get_method("VisualServer", "particles_set_use_local_coordinates");
	___mb.mb_reflection_probe_create = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_create");
	___mb.mb_reflection_probe_set_as_interior = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_as_interior");
	___mb.mb_reflection_probe_set_cull_mask = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_cull_mask");
	___mb.mb_reflection_probe_set_enable_box_projection = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_enable_box_projection");
	___mb.mb_reflection_probe_set_enable_shadows = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_enable_shadows");
	___mb.mb_reflection_probe_set_extents = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_extents");
	___mb.mb_reflection_probe_set_intensity = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_intensity");
	___mb.mb_reflection_probe_set_interior_ambient = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_interior_ambient");
	___mb.mb_reflection_probe_set_interior_ambient_energy = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_interior_ambient_energy");
	___mb.mb_reflection_probe_set_interior_ambient_probe_contribution = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_interior_ambient_probe_contribution");
	___mb.mb_reflection_probe_set_max_distance = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_max_distance");
	___mb.mb_reflection_probe_set_origin_offset = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_origin_offset");
	___mb.mb_reflection_probe_set_update_mode = godot::api->godot_method_bind_get_method("VisualServer", "reflection_probe_set_update_mode");
	___mb.mb_request_frame_drawn_callback = godot::api->godot_method_bind_get_method("VisualServer", "request_frame_drawn_callback");
	___mb.mb_scenario_create = godot::api->godot_method_bind_get_method("VisualServer", "scenario_create");
	___mb.mb_scenario_set_debug = godot::api->godot_method_bind_get_method("VisualServer", "scenario_set_debug");
	___mb.mb_scenario_set_environment = godot::api->godot_method_bind_get_method("VisualServer", "scenario_set_environment");
	___mb.mb_scenario_set_fallback_environment = godot::api->godot_method_bind_get_method("VisualServer", "scenario_set_fallback_environment");
	___mb.mb_scenario_set_reflection_atlas_size = godot::api->godot_method_bind_get_method("VisualServer", "scenario_set_reflection_atlas_size");
	___mb.mb_set_boot_image = godot::api->godot_method_bind_get_method("VisualServer", "set_boot_image");
	___mb.mb_set_debug_generate_wireframes = godot::api->godot_method_bind_get_method("VisualServer", "set_debug_generate_wireframes");
	___mb.mb_set_default_clear_color = godot::api->godot_method_bind_get_method("VisualServer", "set_default_clear_color");
	___mb.mb_shader_create = godot::api->godot_method_bind_get_method("VisualServer", "shader_create");
	___mb.mb_shader_get_code = godot::api->godot_method_bind_get_method("VisualServer", "shader_get_code");
	___mb.mb_shader_get_default_texture_param = godot::api->godot_method_bind_get_method("VisualServer", "shader_get_default_texture_param");
	___mb.mb_shader_get_param_list = godot::api->godot_method_bind_get_method("VisualServer", "shader_get_param_list");
	___mb.mb_shader_set_code = godot::api->godot_method_bind_get_method("VisualServer", "shader_set_code");
	___mb.mb_shader_set_default_texture_param = godot::api->godot_method_bind_get_method("VisualServer", "shader_set_default_texture_param");
	___mb.mb_skeleton_allocate = godot::api->godot_method_bind_get_method("VisualServer", "skeleton_allocate");
	___mb.mb_skeleton_bone_get_transform = godot::api->godot_method_bind_get_method("VisualServer", "skeleton_bone_get_transform");
	___mb.mb_skeleton_bone_get_transform_2d = godot::api->godot_method_bind_get_method("VisualServer", "skeleton_bone_get_transform_2d");
	___mb.mb_skeleton_bone_set_transform = godot::api->godot_method_bind_get_method("VisualServer", "skeleton_bone_set_transform");
	___mb.mb_skeleton_bone_set_transform_2d = godot::api->godot_method_bind_get_method("VisualServer", "skeleton_bone_set_transform_2d");
	___mb.mb_skeleton_create = godot::api->godot_method_bind_get_method("VisualServer", "skeleton_create");
	___mb.mb_skeleton_get_bone_count = godot::api->godot_method_bind_get_method("VisualServer", "skeleton_get_bone_count");
	___mb.mb_sky_create = godot::api->godot_method_bind_get_method("VisualServer", "sky_create");
	___mb.mb_sky_set_texture = godot::api->godot_method_bind_get_method("VisualServer", "sky_set_texture");
	___mb.mb_spot_light_create = godot::api->godot_method_bind_get_method("VisualServer", "spot_light_create");
	___mb.mb_sync = godot::api->godot_method_bind_get_method("VisualServer", "sync");
	___mb.mb_texture_allocate = godot::api->godot_method_bind_get_method("VisualServer", "texture_allocate");
	___mb.mb_texture_bind = godot::api->godot_method_bind_get_method("VisualServer", "texture_bind");
	___mb.mb_texture_create = godot::api->godot_method_bind_get_method("VisualServer", "texture_create");
	___mb.mb_texture_create_from_image = godot::api->godot_method_bind_get_method("VisualServer", "texture_create_from_image");
	___mb.mb_texture_debug_usage = godot::api->godot_method_bind_get_method("VisualServer", "texture_debug_usage");
	___mb.mb_texture_get_data = godot::api->godot_method_bind_get_method("VisualServer", "texture_get_data");
	___mb.mb_texture_get_depth = godot::api->godot_method_bind_get_method("VisualServer", "texture_get_depth");
	___mb.mb_texture_get_flags = godot::api->godot_method_bind_get_method("VisualServer", "texture_get_flags");
	___mb.mb_texture_get_format = godot::api->godot_method_bind_get_method("VisualServer", "texture_get_format");
	___mb.mb_texture_get_height = godot::api->godot_method_bind_get_method("VisualServer", "texture_get_height");
	___mb.mb_texture_get_path = godot::api->godot_method_bind_get_method("VisualServer", "texture_get_path");
	___mb.mb_texture_get_texid = godot::api->godot_method_bind_get_method("VisualServer", "texture_get_texid");
	___mb.mb_texture_get_type = godot::api->godot_method_bind_get_method("VisualServer", "texture_get_type");
	___mb.mb_texture_get_width = godot::api->godot_method_bind_get_method("VisualServer", "texture_get_width");
	___mb.mb_texture_set_data = godot::api->godot_method_bind_get_method("VisualServer", "texture_set_data");
	___mb.mb_texture_set_data_partial = godot::api->godot_method_bind_get_method("VisualServer", "texture_set_data_partial");
	___mb.mb_texture_set_flags = godot::api->godot_method_bind_get_method("VisualServer", "texture_set_flags");
	___mb.mb_texture_set_path = godot::api->godot_method_bind_get_method("VisualServer", "texture_set_path");
	___mb.mb_texture_set_shrink_all_x2_on_set_data = godot::api->godot_method_bind_get_method("VisualServer", "texture_set_shrink_all_x2_on_set_data");
	___mb.mb_texture_set_size_override = godot::api->godot_method_bind_get_method("VisualServer", "texture_set_size_override");
	___mb.mb_textures_keep_original = godot::api->godot_method_bind_get_method("VisualServer", "textures_keep_original");
	___mb.mb_viewport_attach_camera = godot::api->godot_method_bind_get_method("VisualServer", "viewport_attach_camera");
	___mb.mb_viewport_attach_canvas = godot::api->godot_method_bind_get_method("VisualServer", "viewport_attach_canvas");
	___mb.mb_viewport_attach_to_screen = godot::api->godot_method_bind_get_method("VisualServer", "viewport_attach_to_screen");
	___mb.mb_viewport_create = godot::api->godot_method_bind_get_method("VisualServer", "viewport_create");
	___mb.mb_viewport_detach = godot::api->godot_method_bind_get_method("VisualServer", "viewport_detach");
	___mb.mb_viewport_get_render_info = godot::api->godot_method_bind_get_method("VisualServer", "viewport_get_render_info");
	___mb.mb_viewport_get_texture = godot::api->godot_method_bind_get_method("VisualServer", "viewport_get_texture");
	___mb.mb_viewport_remove_canvas = godot::api->godot_method_bind_get_method("VisualServer", "viewport_remove_canvas");
	___mb.mb_viewport_set_active = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_active");
	___mb.mb_viewport_set_canvas_stacking = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_canvas_stacking");
	___mb.mb_viewport_set_canvas_transform = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_canvas_transform");
	___mb.mb_viewport_set_clear_mode = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_clear_mode");
	___mb.mb_viewport_set_debug_draw = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_debug_draw");
	___mb.mb_viewport_set_disable_3d = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_disable_3d");
	___mb.mb_viewport_set_disable_environment = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_disable_environment");
	___mb.mb_viewport_set_global_canvas_transform = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_global_canvas_transform");
	___mb.mb_viewport_set_hdr = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_hdr");
	___mb.mb_viewport_set_hide_canvas = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_hide_canvas");
	___mb.mb_viewport_set_hide_scenario = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_hide_scenario");
	___mb.mb_viewport_set_msaa = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_msaa");
	___mb.mb_viewport_set_parent_viewport = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_parent_viewport");
	___mb.mb_viewport_set_render_direct_to_screen = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_render_direct_to_screen");
	___mb.mb_viewport_set_scenario = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_scenario");
	___mb.mb_viewport_set_shadow_atlas_quadrant_subdivision = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_shadow_atlas_quadrant_subdivision");
	___mb.mb_viewport_set_shadow_atlas_size = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_shadow_atlas_size");
	___mb.mb_viewport_set_size = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_size");
	___mb.mb_viewport_set_transparent_background = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_transparent_background");
	___mb.mb_viewport_set_update_mode = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_update_mode");
	___mb.mb_viewport_set_usage = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_usage");
	___mb.mb_viewport_set_use_arvr = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_use_arvr");
	___mb.mb_viewport_set_vflip = godot::api->godot_method_bind_get_method("VisualServer", "viewport_set_vflip");
}

void VisualServer::black_bars_set_images(const RID left, const RID top, const RID right, const RID bottom) {
	___godot_icall_void_RID_RID_RID_RID(___mb.mb_black_bars_set_images, (const Object *) this, left, top, right, bottom);
}

void VisualServer::black_bars_set_margins(const int64_t left, const int64_t top, const int64_t right, const int64_t bottom) {
	___godot_icall_void_int_int_int_int(___mb.mb_black_bars_set_margins, (const Object *) this, left, top, right, bottom);
}

RID VisualServer::camera_create() {
	return ___godot_icall_RID(___mb.mb_camera_create, (const Object *) this);
}

void VisualServer::camera_set_cull_mask(const RID camera, const int64_t layers) {
	___godot_icall_void_RID_int(___mb.mb_camera_set_cull_mask, (const Object *) this, camera, layers);
}

void VisualServer::camera_set_environment(const RID camera, const RID env) {
	___godot_icall_void_RID_RID(___mb.mb_camera_set_environment, (const Object *) this, camera, env);
}

void VisualServer::camera_set_frustum(const RID camera, const real_t size, const Vector2 offset, const real_t z_near, const real_t z_far) {
	___godot_icall_void_RID_float_Vector2_float_float(___mb.mb_camera_set_frustum, (const Object *) this, camera, size, offset, z_near, z_far);
}

void VisualServer::camera_set_orthogonal(const RID camera, const real_t size, const real_t z_near, const real_t z_far) {
	___godot_icall_void_RID_float_float_float(___mb.mb_camera_set_orthogonal, (const Object *) this, camera, size, z_near, z_far);
}

void VisualServer::camera_set_perspective(const RID camera, const real_t fovy_degrees, const real_t z_near, const real_t z_far) {
	___godot_icall_void_RID_float_float_float(___mb.mb_camera_set_perspective, (const Object *) this, camera, fovy_degrees, z_near, z_far);
}

void VisualServer::camera_set_transform(const RID camera, const Transform transform) {
	___godot_icall_void_RID_Transform(___mb.mb_camera_set_transform, (const Object *) this, camera, transform);
}

void VisualServer::camera_set_use_vertical_aspect(const RID camera, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_camera_set_use_vertical_aspect, (const Object *) this, camera, enable);
}

RID VisualServer::canvas_create() {
	return ___godot_icall_RID(___mb.mb_canvas_create, (const Object *) this);
}

void VisualServer::canvas_item_add_circle(const RID item, const Vector2 pos, const real_t radius, const Color color) {
	___godot_icall_void_RID_Vector2_float_Color(___mb.mb_canvas_item_add_circle, (const Object *) this, item, pos, radius, color);
}

void VisualServer::canvas_item_add_clip_ignore(const RID item, const bool ignore) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_item_add_clip_ignore, (const Object *) this, item, ignore);
}

void VisualServer::canvas_item_add_line(const RID item, const Vector2 from, const Vector2 to, const Color color, const real_t width, const bool antialiased) {
	___godot_icall_void_RID_Vector2_Vector2_Color_float_bool(___mb.mb_canvas_item_add_line, (const Object *) this, item, from, to, color, width, antialiased);
}

void VisualServer::canvas_item_add_mesh(const RID item, const RID mesh, const Transform2D transform, const Color modulate, const RID texture, const RID normal_map) {
	___godot_icall_void_RID_RID_Transform2D_Color_RID_RID(___mb.mb_canvas_item_add_mesh, (const Object *) this, item, mesh, transform, modulate, texture, normal_map);
}

void VisualServer::canvas_item_add_multimesh(const RID item, const RID mesh, const RID texture, const RID normal_map) {
	___godot_icall_void_RID_RID_RID_RID(___mb.mb_canvas_item_add_multimesh, (const Object *) this, item, mesh, texture, normal_map);
}

void VisualServer::canvas_item_add_nine_patch(const RID item, const Rect2 rect, const Rect2 source, const RID texture, const Vector2 topleft, const Vector2 bottomright, const int64_t x_axis_mode, const int64_t y_axis_mode, const bool draw_center, const Color modulate, const RID normal_map) {
	___godot_icall_void_RID_Rect2_Rect2_RID_Vector2_Vector2_int_int_bool_Color_RID(___mb.mb_canvas_item_add_nine_patch, (const Object *) this, item, rect, source, texture, topleft, bottomright, x_axis_mode, y_axis_mode, draw_center, modulate, normal_map);
}

void VisualServer::canvas_item_add_particles(const RID item, const RID particles, const RID texture, const RID normal_map) {
	___godot_icall_void_RID_RID_RID_RID(___mb.mb_canvas_item_add_particles, (const Object *) this, item, particles, texture, normal_map);
}

void VisualServer::canvas_item_add_polygon(const RID item, const PoolVector2Array points, const PoolColorArray colors, const PoolVector2Array uvs, const RID texture, const RID normal_map, const bool antialiased) {
	___godot_icall_void_RID_PoolVector2Array_PoolColorArray_PoolVector2Array_RID_RID_bool(___mb.mb_canvas_item_add_polygon, (const Object *) this, item, points, colors, uvs, texture, normal_map, antialiased);
}

void VisualServer::canvas_item_add_polyline(const RID item, const PoolVector2Array points, const PoolColorArray colors, const real_t width, const bool antialiased) {
	___godot_icall_void_RID_PoolVector2Array_PoolColorArray_float_bool(___mb.mb_canvas_item_add_polyline, (const Object *) this, item, points, colors, width, antialiased);
}

void VisualServer::canvas_item_add_primitive(const RID item, const PoolVector2Array points, const PoolColorArray colors, const PoolVector2Array uvs, const RID texture, const real_t width, const RID normal_map) {
	___godot_icall_void_RID_PoolVector2Array_PoolColorArray_PoolVector2Array_RID_float_RID(___mb.mb_canvas_item_add_primitive, (const Object *) this, item, points, colors, uvs, texture, width, normal_map);
}

void VisualServer::canvas_item_add_rect(const RID item, const Rect2 rect, const Color color) {
	___godot_icall_void_RID_Rect2_Color(___mb.mb_canvas_item_add_rect, (const Object *) this, item, rect, color);
}

void VisualServer::canvas_item_add_set_transform(const RID item, const Transform2D transform) {
	___godot_icall_void_RID_Transform2D(___mb.mb_canvas_item_add_set_transform, (const Object *) this, item, transform);
}

void VisualServer::canvas_item_add_texture_rect(const RID item, const Rect2 rect, const RID texture, const bool tile, const Color modulate, const bool transpose, const RID normal_map) {
	___godot_icall_void_RID_Rect2_RID_bool_Color_bool_RID(___mb.mb_canvas_item_add_texture_rect, (const Object *) this, item, rect, texture, tile, modulate, transpose, normal_map);
}

void VisualServer::canvas_item_add_texture_rect_region(const RID item, const Rect2 rect, const RID texture, const Rect2 src_rect, const Color modulate, const bool transpose, const RID normal_map, const bool clip_uv) {
	___godot_icall_void_RID_Rect2_RID_Rect2_Color_bool_RID_bool(___mb.mb_canvas_item_add_texture_rect_region, (const Object *) this, item, rect, texture, src_rect, modulate, transpose, normal_map, clip_uv);
}

void VisualServer::canvas_item_add_triangle_array(const RID item, const PoolIntArray indices, const PoolVector2Array points, const PoolColorArray colors, const PoolVector2Array uvs, const PoolIntArray bones, const PoolRealArray weights, const RID texture, const int64_t count, const RID normal_map, const bool antialiased, const bool antialiasing_use_indices) {
	___godot_icall_void_RID_PoolIntArray_PoolVector2Array_PoolColorArray_PoolVector2Array_PoolIntArray_PoolRealArray_RID_int_RID_bool_bool(___mb.mb_canvas_item_add_triangle_array, (const Object *) this, item, indices, points, colors, uvs, bones, weights, texture, count, normal_map, antialiased, antialiasing_use_indices);
}

void VisualServer::canvas_item_clear(const RID item) {
	___godot_icall_void_RID(___mb.mb_canvas_item_clear, (const Object *) this, item);
}

RID VisualServer::canvas_item_create() {
	return ___godot_icall_RID(___mb.mb_canvas_item_create, (const Object *) this);
}

void VisualServer::canvas_item_set_clip(const RID item, const bool clip) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_item_set_clip, (const Object *) this, item, clip);
}

void VisualServer::canvas_item_set_copy_to_backbuffer(const RID item, const bool enabled, const Rect2 rect) {
	___godot_icall_void_RID_bool_Rect2(___mb.mb_canvas_item_set_copy_to_backbuffer, (const Object *) this, item, enabled, rect);
}

void VisualServer::canvas_item_set_custom_rect(const RID item, const bool use_custom_rect, const Rect2 rect) {
	___godot_icall_void_RID_bool_Rect2(___mb.mb_canvas_item_set_custom_rect, (const Object *) this, item, use_custom_rect, rect);
}

void VisualServer::canvas_item_set_distance_field_mode(const RID item, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_item_set_distance_field_mode, (const Object *) this, item, enabled);
}

void VisualServer::canvas_item_set_draw_behind_parent(const RID item, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_item_set_draw_behind_parent, (const Object *) this, item, enabled);
}

void VisualServer::canvas_item_set_draw_index(const RID item, const int64_t index) {
	___godot_icall_void_RID_int(___mb.mb_canvas_item_set_draw_index, (const Object *) this, item, index);
}

void VisualServer::canvas_item_set_light_mask(const RID item, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_canvas_item_set_light_mask, (const Object *) this, item, mask);
}

void VisualServer::canvas_item_set_material(const RID item, const RID material) {
	___godot_icall_void_RID_RID(___mb.mb_canvas_item_set_material, (const Object *) this, item, material);
}

void VisualServer::canvas_item_set_modulate(const RID item, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_canvas_item_set_modulate, (const Object *) this, item, color);
}

void VisualServer::canvas_item_set_parent(const RID item, const RID parent) {
	___godot_icall_void_RID_RID(___mb.mb_canvas_item_set_parent, (const Object *) this, item, parent);
}

void VisualServer::canvas_item_set_self_modulate(const RID item, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_canvas_item_set_self_modulate, (const Object *) this, item, color);
}

void VisualServer::canvas_item_set_sort_children_by_y(const RID item, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_item_set_sort_children_by_y, (const Object *) this, item, enabled);
}

void VisualServer::canvas_item_set_transform(const RID item, const Transform2D transform) {
	___godot_icall_void_RID_Transform2D(___mb.mb_canvas_item_set_transform, (const Object *) this, item, transform);
}

void VisualServer::canvas_item_set_use_parent_material(const RID item, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_item_set_use_parent_material, (const Object *) this, item, enabled);
}

void VisualServer::canvas_item_set_visible(const RID item, const bool visible) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_item_set_visible, (const Object *) this, item, visible);
}

void VisualServer::canvas_item_set_z_as_relative_to_parent(const RID item, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_item_set_z_as_relative_to_parent, (const Object *) this, item, enabled);
}

void VisualServer::canvas_item_set_z_index(const RID item, const int64_t z_index) {
	___godot_icall_void_RID_int(___mb.mb_canvas_item_set_z_index, (const Object *) this, item, z_index);
}

void VisualServer::canvas_light_attach_to_canvas(const RID light, const RID canvas) {
	___godot_icall_void_RID_RID(___mb.mb_canvas_light_attach_to_canvas, (const Object *) this, light, canvas);
}

RID VisualServer::canvas_light_create() {
	return ___godot_icall_RID(___mb.mb_canvas_light_create, (const Object *) this);
}

void VisualServer::canvas_light_occluder_attach_to_canvas(const RID occluder, const RID canvas) {
	___godot_icall_void_RID_RID(___mb.mb_canvas_light_occluder_attach_to_canvas, (const Object *) this, occluder, canvas);
}

RID VisualServer::canvas_light_occluder_create() {
	return ___godot_icall_RID(___mb.mb_canvas_light_occluder_create, (const Object *) this);
}

void VisualServer::canvas_light_occluder_set_enabled(const RID occluder, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_light_occluder_set_enabled, (const Object *) this, occluder, enabled);
}

void VisualServer::canvas_light_occluder_set_light_mask(const RID occluder, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_canvas_light_occluder_set_light_mask, (const Object *) this, occluder, mask);
}

void VisualServer::canvas_light_occluder_set_polygon(const RID occluder, const RID polygon) {
	___godot_icall_void_RID_RID(___mb.mb_canvas_light_occluder_set_polygon, (const Object *) this, occluder, polygon);
}

void VisualServer::canvas_light_occluder_set_transform(const RID occluder, const Transform2D transform) {
	___godot_icall_void_RID_Transform2D(___mb.mb_canvas_light_occluder_set_transform, (const Object *) this, occluder, transform);
}

void VisualServer::canvas_light_set_color(const RID light, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_canvas_light_set_color, (const Object *) this, light, color);
}

void VisualServer::canvas_light_set_enabled(const RID light, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_light_set_enabled, (const Object *) this, light, enabled);
}

void VisualServer::canvas_light_set_energy(const RID light, const real_t energy) {
	___godot_icall_void_RID_float(___mb.mb_canvas_light_set_energy, (const Object *) this, light, energy);
}

void VisualServer::canvas_light_set_height(const RID light, const real_t height) {
	___godot_icall_void_RID_float(___mb.mb_canvas_light_set_height, (const Object *) this, light, height);
}

void VisualServer::canvas_light_set_item_cull_mask(const RID light, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_canvas_light_set_item_cull_mask, (const Object *) this, light, mask);
}

void VisualServer::canvas_light_set_item_shadow_cull_mask(const RID light, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_canvas_light_set_item_shadow_cull_mask, (const Object *) this, light, mask);
}

void VisualServer::canvas_light_set_layer_range(const RID light, const int64_t min_layer, const int64_t max_layer) {
	___godot_icall_void_RID_int_int(___mb.mb_canvas_light_set_layer_range, (const Object *) this, light, min_layer, max_layer);
}

void VisualServer::canvas_light_set_mode(const RID light, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_canvas_light_set_mode, (const Object *) this, light, mode);
}

void VisualServer::canvas_light_set_scale(const RID light, const real_t scale) {
	___godot_icall_void_RID_float(___mb.mb_canvas_light_set_scale, (const Object *) this, light, scale);
}

void VisualServer::canvas_light_set_shadow_buffer_size(const RID light, const int64_t size) {
	___godot_icall_void_RID_int(___mb.mb_canvas_light_set_shadow_buffer_size, (const Object *) this, light, size);
}

void VisualServer::canvas_light_set_shadow_color(const RID light, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_canvas_light_set_shadow_color, (const Object *) this, light, color);
}

void VisualServer::canvas_light_set_shadow_enabled(const RID light, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_canvas_light_set_shadow_enabled, (const Object *) this, light, enabled);
}

void VisualServer::canvas_light_set_shadow_filter(const RID light, const int64_t filter) {
	___godot_icall_void_RID_int(___mb.mb_canvas_light_set_shadow_filter, (const Object *) this, light, filter);
}

void VisualServer::canvas_light_set_shadow_gradient_length(const RID light, const real_t length) {
	___godot_icall_void_RID_float(___mb.mb_canvas_light_set_shadow_gradient_length, (const Object *) this, light, length);
}

void VisualServer::canvas_light_set_shadow_smooth(const RID light, const real_t smooth) {
	___godot_icall_void_RID_float(___mb.mb_canvas_light_set_shadow_smooth, (const Object *) this, light, smooth);
}

void VisualServer::canvas_light_set_texture(const RID light, const RID texture) {
	___godot_icall_void_RID_RID(___mb.mb_canvas_light_set_texture, (const Object *) this, light, texture);
}

void VisualServer::canvas_light_set_texture_offset(const RID light, const Vector2 offset) {
	___godot_icall_void_RID_Vector2(___mb.mb_canvas_light_set_texture_offset, (const Object *) this, light, offset);
}

void VisualServer::canvas_light_set_transform(const RID light, const Transform2D transform) {
	___godot_icall_void_RID_Transform2D(___mb.mb_canvas_light_set_transform, (const Object *) this, light, transform);
}

void VisualServer::canvas_light_set_z_range(const RID light, const int64_t min_z, const int64_t max_z) {
	___godot_icall_void_RID_int_int(___mb.mb_canvas_light_set_z_range, (const Object *) this, light, min_z, max_z);
}

RID VisualServer::canvas_occluder_polygon_create() {
	return ___godot_icall_RID(___mb.mb_canvas_occluder_polygon_create, (const Object *) this);
}

void VisualServer::canvas_occluder_polygon_set_cull_mode(const RID occluder_polygon, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_canvas_occluder_polygon_set_cull_mode, (const Object *) this, occluder_polygon, mode);
}

void VisualServer::canvas_occluder_polygon_set_shape(const RID occluder_polygon, const PoolVector2Array shape, const bool closed) {
	___godot_icall_void_RID_PoolVector2Array_bool(___mb.mb_canvas_occluder_polygon_set_shape, (const Object *) this, occluder_polygon, shape, closed);
}

void VisualServer::canvas_occluder_polygon_set_shape_as_lines(const RID occluder_polygon, const PoolVector2Array shape) {
	___godot_icall_void_RID_PoolVector2Array(___mb.mb_canvas_occluder_polygon_set_shape_as_lines, (const Object *) this, occluder_polygon, shape);
}

void VisualServer::canvas_set_item_mirroring(const RID canvas, const RID item, const Vector2 mirroring) {
	___godot_icall_void_RID_RID_Vector2(___mb.mb_canvas_set_item_mirroring, (const Object *) this, canvas, item, mirroring);
}

void VisualServer::canvas_set_modulate(const RID canvas, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_canvas_set_modulate, (const Object *) this, canvas, color);
}

RID VisualServer::directional_light_create() {
	return ___godot_icall_RID(___mb.mb_directional_light_create, (const Object *) this);
}

void VisualServer::draw(const bool swap_buffers, const real_t frame_step) {
	___godot_icall_void_bool_float(___mb.mb_draw, (const Object *) this, swap_buffers, frame_step);
}

RID VisualServer::environment_create() {
	return ___godot_icall_RID(___mb.mb_environment_create, (const Object *) this);
}

void VisualServer::environment_set_adjustment(const RID env, const bool enable, const real_t brightness, const real_t contrast, const real_t saturation, const RID ramp) {
	___godot_icall_void_RID_bool_float_float_float_RID(___mb.mb_environment_set_adjustment, (const Object *) this, env, enable, brightness, contrast, saturation, ramp);
}

void VisualServer::environment_set_ambient_light(const RID env, const Color color, const real_t energy, const real_t sky_contibution) {
	___godot_icall_void_RID_Color_float_float(___mb.mb_environment_set_ambient_light, (const Object *) this, env, color, energy, sky_contibution);
}

void VisualServer::environment_set_background(const RID env, const int64_t bg) {
	___godot_icall_void_RID_int(___mb.mb_environment_set_background, (const Object *) this, env, bg);
}

void VisualServer::environment_set_bg_color(const RID env, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_environment_set_bg_color, (const Object *) this, env, color);
}

void VisualServer::environment_set_bg_energy(const RID env, const real_t energy) {
	___godot_icall_void_RID_float(___mb.mb_environment_set_bg_energy, (const Object *) this, env, energy);
}

void VisualServer::environment_set_canvas_max_layer(const RID env, const int64_t max_layer) {
	___godot_icall_void_RID_int(___mb.mb_environment_set_canvas_max_layer, (const Object *) this, env, max_layer);
}

void VisualServer::environment_set_dof_blur_far(const RID env, const bool enable, const real_t distance, const real_t transition, const real_t far_amount, const int64_t quality) {
	___godot_icall_void_RID_bool_float_float_float_int(___mb.mb_environment_set_dof_blur_far, (const Object *) this, env, enable, distance, transition, far_amount, quality);
}

void VisualServer::environment_set_dof_blur_near(const RID env, const bool enable, const real_t distance, const real_t transition, const real_t far_amount, const int64_t quality) {
	___godot_icall_void_RID_bool_float_float_float_int(___mb.mb_environment_set_dof_blur_near, (const Object *) this, env, enable, distance, transition, far_amount, quality);
}

void VisualServer::environment_set_fog(const RID env, const bool enable, const Color color, const Color sun_color, const real_t sun_amount) {
	___godot_icall_void_RID_bool_Color_Color_float(___mb.mb_environment_set_fog, (const Object *) this, env, enable, color, sun_color, sun_amount);
}

void VisualServer::environment_set_fog_depth(const RID env, const bool enable, const real_t depth_begin, const real_t depth_end, const real_t depth_curve, const bool transmit, const real_t transmit_curve) {
	___godot_icall_void_RID_bool_float_float_float_bool_float(___mb.mb_environment_set_fog_depth, (const Object *) this, env, enable, depth_begin, depth_end, depth_curve, transmit, transmit_curve);
}

void VisualServer::environment_set_fog_height(const RID env, const bool enable, const real_t min_height, const real_t max_height, const real_t height_curve) {
	___godot_icall_void_RID_bool_float_float_float(___mb.mb_environment_set_fog_height, (const Object *) this, env, enable, min_height, max_height, height_curve);
}

void VisualServer::environment_set_glow(const RID env, const bool enable, const int64_t level_flags, const real_t intensity, const real_t strength, const real_t bloom_threshold, const int64_t blend_mode, const real_t hdr_bleed_threshold, const real_t hdr_bleed_scale, const real_t hdr_luminance_cap, const bool bicubic_upscale) {
	___godot_icall_void_RID_bool_int_float_float_float_int_float_float_float_bool(___mb.mb_environment_set_glow, (const Object *) this, env, enable, level_flags, intensity, strength, bloom_threshold, blend_mode, hdr_bleed_threshold, hdr_bleed_scale, hdr_luminance_cap, bicubic_upscale);
}

void VisualServer::environment_set_sky(const RID env, const RID sky) {
	___godot_icall_void_RID_RID(___mb.mb_environment_set_sky, (const Object *) this, env, sky);
}

void VisualServer::environment_set_sky_custom_fov(const RID env, const real_t scale) {
	___godot_icall_void_RID_float(___mb.mb_environment_set_sky_custom_fov, (const Object *) this, env, scale);
}

void VisualServer::environment_set_sky_orientation(const RID env, const Basis orientation) {
	___godot_icall_void_RID_Basis(___mb.mb_environment_set_sky_orientation, (const Object *) this, env, orientation);
}

void VisualServer::environment_set_ssao(const RID env, const bool enable, const real_t radius, const real_t intensity, const real_t radius2, const real_t intensity2, const real_t bias, const real_t light_affect, const real_t ao_channel_affect, const Color color, const int64_t quality, const int64_t blur, const real_t bilateral_sharpness) {
	___godot_icall_void_RID_bool_float_float_float_float_float_float_float_Color_int_int_float(___mb.mb_environment_set_ssao, (const Object *) this, env, enable, radius, intensity, radius2, intensity2, bias, light_affect, ao_channel_affect, color, quality, blur, bilateral_sharpness);
}

void VisualServer::environment_set_ssr(const RID env, const bool enable, const int64_t max_steps, const real_t fade_in, const real_t fade_out, const real_t depth_tolerance, const bool roughness) {
	___godot_icall_void_RID_bool_int_float_float_float_bool(___mb.mb_environment_set_ssr, (const Object *) this, env, enable, max_steps, fade_in, fade_out, depth_tolerance, roughness);
}

void VisualServer::environment_set_tonemap(const RID env, const int64_t tone_mapper, const real_t exposure, const real_t white, const bool auto_exposure, const real_t min_luminance, const real_t max_luminance, const real_t auto_exp_speed, const real_t auto_exp_grey) {
	___godot_icall_void_RID_int_float_float_bool_float_float_float_float(___mb.mb_environment_set_tonemap, (const Object *) this, env, tone_mapper, exposure, white, auto_exposure, min_luminance, max_luminance, auto_exp_speed, auto_exp_grey);
}

void VisualServer::finish() {
	___godot_icall_void(___mb.mb_finish, (const Object *) this);
}

void VisualServer::force_draw(const bool swap_buffers, const real_t frame_step) {
	___godot_icall_void_bool_float(___mb.mb_force_draw, (const Object *) this, swap_buffers, frame_step);
}

void VisualServer::force_sync() {
	___godot_icall_void(___mb.mb_force_sync, (const Object *) this);
}

void VisualServer::free_rid(const RID rid) {
	___godot_icall_void_RID(___mb.mb_free_rid, (const Object *) this, rid);
}

int64_t VisualServer::get_render_info(const int64_t info) {
	return ___godot_icall_int_int(___mb.mb_get_render_info, (const Object *) this, info);
}

RID VisualServer::get_test_cube() {
	return ___godot_icall_RID(___mb.mb_get_test_cube, (const Object *) this);
}

RID VisualServer::get_test_texture() {
	return ___godot_icall_RID(___mb.mb_get_test_texture, (const Object *) this);
}

String VisualServer::get_video_adapter_name() const {
	return ___godot_icall_String(___mb.mb_get_video_adapter_name, (const Object *) this);
}

String VisualServer::get_video_adapter_vendor() const {
	return ___godot_icall_String(___mb.mb_get_video_adapter_vendor, (const Object *) this);
}

RID VisualServer::get_white_texture() {
	return ___godot_icall_RID(___mb.mb_get_white_texture, (const Object *) this);
}

RID VisualServer::gi_probe_create() {
	return ___godot_icall_RID(___mb.mb_gi_probe_create, (const Object *) this);
}

real_t VisualServer::gi_probe_get_bias(const RID probe) const {
	return ___godot_icall_float_RID(___mb.mb_gi_probe_get_bias, (const Object *) this, probe);
}

AABB VisualServer::gi_probe_get_bounds(const RID probe) const {
	return ___godot_icall_AABB_RID(___mb.mb_gi_probe_get_bounds, (const Object *) this, probe);
}

real_t VisualServer::gi_probe_get_cell_size(const RID probe) const {
	return ___godot_icall_float_RID(___mb.mb_gi_probe_get_cell_size, (const Object *) this, probe);
}

PoolIntArray VisualServer::gi_probe_get_dynamic_data(const RID probe) const {
	return ___godot_icall_PoolIntArray_RID(___mb.mb_gi_probe_get_dynamic_data, (const Object *) this, probe);
}

int64_t VisualServer::gi_probe_get_dynamic_range(const RID probe) const {
	return ___godot_icall_int_RID(___mb.mb_gi_probe_get_dynamic_range, (const Object *) this, probe);
}

real_t VisualServer::gi_probe_get_energy(const RID probe) const {
	return ___godot_icall_float_RID(___mb.mb_gi_probe_get_energy, (const Object *) this, probe);
}

real_t VisualServer::gi_probe_get_normal_bias(const RID probe) const {
	return ___godot_icall_float_RID(___mb.mb_gi_probe_get_normal_bias, (const Object *) this, probe);
}

real_t VisualServer::gi_probe_get_propagation(const RID probe) const {
	return ___godot_icall_float_RID(___mb.mb_gi_probe_get_propagation, (const Object *) this, probe);
}

Transform VisualServer::gi_probe_get_to_cell_xform(const RID probe) const {
	return ___godot_icall_Transform_RID(___mb.mb_gi_probe_get_to_cell_xform, (const Object *) this, probe);
}

bool VisualServer::gi_probe_is_compressed(const RID probe) const {
	return ___godot_icall_bool_RID(___mb.mb_gi_probe_is_compressed, (const Object *) this, probe);
}

bool VisualServer::gi_probe_is_interior(const RID probe) const {
	return ___godot_icall_bool_RID(___mb.mb_gi_probe_is_interior, (const Object *) this, probe);
}

void VisualServer::gi_probe_set_bias(const RID probe, const real_t bias) {
	___godot_icall_void_RID_float(___mb.mb_gi_probe_set_bias, (const Object *) this, probe, bias);
}

void VisualServer::gi_probe_set_bounds(const RID probe, const AABB bounds) {
	___godot_icall_void_RID_AABB(___mb.mb_gi_probe_set_bounds, (const Object *) this, probe, bounds);
}

void VisualServer::gi_probe_set_cell_size(const RID probe, const real_t range) {
	___godot_icall_void_RID_float(___mb.mb_gi_probe_set_cell_size, (const Object *) this, probe, range);
}

void VisualServer::gi_probe_set_compress(const RID probe, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_gi_probe_set_compress, (const Object *) this, probe, enable);
}

void VisualServer::gi_probe_set_dynamic_data(const RID probe, const PoolIntArray data) {
	___godot_icall_void_RID_PoolIntArray(___mb.mb_gi_probe_set_dynamic_data, (const Object *) this, probe, data);
}

void VisualServer::gi_probe_set_dynamic_range(const RID probe, const int64_t range) {
	___godot_icall_void_RID_int(___mb.mb_gi_probe_set_dynamic_range, (const Object *) this, probe, range);
}

void VisualServer::gi_probe_set_energy(const RID probe, const real_t energy) {
	___godot_icall_void_RID_float(___mb.mb_gi_probe_set_energy, (const Object *) this, probe, energy);
}

void VisualServer::gi_probe_set_interior(const RID probe, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_gi_probe_set_interior, (const Object *) this, probe, enable);
}

void VisualServer::gi_probe_set_normal_bias(const RID probe, const real_t bias) {
	___godot_icall_void_RID_float(___mb.mb_gi_probe_set_normal_bias, (const Object *) this, probe, bias);
}

void VisualServer::gi_probe_set_propagation(const RID probe, const real_t propagation) {
	___godot_icall_void_RID_float(___mb.mb_gi_probe_set_propagation, (const Object *) this, probe, propagation);
}

void VisualServer::gi_probe_set_to_cell_xform(const RID probe, const Transform xform) {
	___godot_icall_void_RID_Transform(___mb.mb_gi_probe_set_to_cell_xform, (const Object *) this, probe, xform);
}

bool VisualServer::has_changed() const {
	return ___godot_icall_bool(___mb.mb_has_changed, (const Object *) this);
}

bool VisualServer::has_feature(const int64_t feature) const {
	return ___godot_icall_bool_int(___mb.mb_has_feature, (const Object *) this, feature);
}

bool VisualServer::has_os_feature(const String feature) const {
	return ___godot_icall_bool_String(___mb.mb_has_os_feature, (const Object *) this, feature);
}

void VisualServer::immediate_begin(const RID immediate, const int64_t primitive, const RID texture) {
	___godot_icall_void_RID_int_RID(___mb.mb_immediate_begin, (const Object *) this, immediate, primitive, texture);
}

void VisualServer::immediate_clear(const RID immediate) {
	___godot_icall_void_RID(___mb.mb_immediate_clear, (const Object *) this, immediate);
}

void VisualServer::immediate_color(const RID immediate, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_immediate_color, (const Object *) this, immediate, color);
}

RID VisualServer::immediate_create() {
	return ___godot_icall_RID(___mb.mb_immediate_create, (const Object *) this);
}

void VisualServer::immediate_end(const RID immediate) {
	___godot_icall_void_RID(___mb.mb_immediate_end, (const Object *) this, immediate);
}

RID VisualServer::immediate_get_material(const RID immediate) const {
	return ___godot_icall_RID_RID(___mb.mb_immediate_get_material, (const Object *) this, immediate);
}

void VisualServer::immediate_normal(const RID immediate, const Vector3 normal) {
	___godot_icall_void_RID_Vector3(___mb.mb_immediate_normal, (const Object *) this, immediate, normal);
}

void VisualServer::immediate_set_material(const RID immediate, const RID material) {
	___godot_icall_void_RID_RID(___mb.mb_immediate_set_material, (const Object *) this, immediate, material);
}

void VisualServer::immediate_tangent(const RID immediate, const Plane tangent) {
	___godot_icall_void_RID_Plane(___mb.mb_immediate_tangent, (const Object *) this, immediate, tangent);
}

void VisualServer::immediate_uv(const RID immediate, const Vector2 tex_uv) {
	___godot_icall_void_RID_Vector2(___mb.mb_immediate_uv, (const Object *) this, immediate, tex_uv);
}

void VisualServer::immediate_uv2(const RID immediate, const Vector2 tex_uv) {
	___godot_icall_void_RID_Vector2(___mb.mb_immediate_uv2, (const Object *) this, immediate, tex_uv);
}

void VisualServer::immediate_vertex(const RID immediate, const Vector3 vertex) {
	___godot_icall_void_RID_Vector3(___mb.mb_immediate_vertex, (const Object *) this, immediate, vertex);
}

void VisualServer::immediate_vertex_2d(const RID immediate, const Vector2 vertex) {
	___godot_icall_void_RID_Vector2(___mb.mb_immediate_vertex_2d, (const Object *) this, immediate, vertex);
}

void VisualServer::init() {
	___godot_icall_void(___mb.mb_init, (const Object *) this);
}

void VisualServer::instance_attach_object_instance_id(const RID instance, const int64_t id) {
	___godot_icall_void_RID_int(___mb.mb_instance_attach_object_instance_id, (const Object *) this, instance, id);
}

void VisualServer::instance_attach_skeleton(const RID instance, const RID skeleton) {
	___godot_icall_void_RID_RID(___mb.mb_instance_attach_skeleton, (const Object *) this, instance, skeleton);
}

RID VisualServer::instance_create() {
	return ___godot_icall_RID(___mb.mb_instance_create, (const Object *) this);
}

RID VisualServer::instance_create2(const RID base, const RID scenario) {
	return ___godot_icall_RID_RID_RID(___mb.mb_instance_create2, (const Object *) this, base, scenario);
}

void VisualServer::instance_geometry_set_as_instance_lod(const RID instance, const RID as_lod_of_instance) {
	___godot_icall_void_RID_RID(___mb.mb_instance_geometry_set_as_instance_lod, (const Object *) this, instance, as_lod_of_instance);
}

void VisualServer::instance_geometry_set_cast_shadows_setting(const RID instance, const int64_t shadow_casting_setting) {
	___godot_icall_void_RID_int(___mb.mb_instance_geometry_set_cast_shadows_setting, (const Object *) this, instance, shadow_casting_setting);
}

void VisualServer::instance_geometry_set_draw_range(const RID instance, const real_t min, const real_t max, const real_t min_margin, const real_t max_margin) {
	___godot_icall_void_RID_float_float_float_float(___mb.mb_instance_geometry_set_draw_range, (const Object *) this, instance, min, max, min_margin, max_margin);
}

void VisualServer::instance_geometry_set_flag(const RID instance, const int64_t flag, const bool enabled) {
	___godot_icall_void_RID_int_bool(___mb.mb_instance_geometry_set_flag, (const Object *) this, instance, flag, enabled);
}

void VisualServer::instance_geometry_set_material_override(const RID instance, const RID material) {
	___godot_icall_void_RID_RID(___mb.mb_instance_geometry_set_material_override, (const Object *) this, instance, material);
}

void VisualServer::instance_set_base(const RID instance, const RID base) {
	___godot_icall_void_RID_RID(___mb.mb_instance_set_base, (const Object *) this, instance, base);
}

void VisualServer::instance_set_blend_shape_weight(const RID instance, const int64_t shape, const real_t weight) {
	___godot_icall_void_RID_int_float(___mb.mb_instance_set_blend_shape_weight, (const Object *) this, instance, shape, weight);
}

void VisualServer::instance_set_custom_aabb(const RID instance, const AABB aabb) {
	___godot_icall_void_RID_AABB(___mb.mb_instance_set_custom_aabb, (const Object *) this, instance, aabb);
}

void VisualServer::instance_set_exterior(const RID instance, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_instance_set_exterior, (const Object *) this, instance, enabled);
}

void VisualServer::instance_set_extra_visibility_margin(const RID instance, const real_t margin) {
	___godot_icall_void_RID_float(___mb.mb_instance_set_extra_visibility_margin, (const Object *) this, instance, margin);
}

void VisualServer::instance_set_layer_mask(const RID instance, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_instance_set_layer_mask, (const Object *) this, instance, mask);
}

void VisualServer::instance_set_scenario(const RID instance, const RID scenario) {
	___godot_icall_void_RID_RID(___mb.mb_instance_set_scenario, (const Object *) this, instance, scenario);
}

void VisualServer::instance_set_surface_material(const RID instance, const int64_t surface, const RID material) {
	___godot_icall_void_RID_int_RID(___mb.mb_instance_set_surface_material, (const Object *) this, instance, surface, material);
}

void VisualServer::instance_set_transform(const RID instance, const Transform transform) {
	___godot_icall_void_RID_Transform(___mb.mb_instance_set_transform, (const Object *) this, instance, transform);
}

void VisualServer::instance_set_use_lightmap(const RID instance, const RID lightmap_instance, const RID lightmap) {
	___godot_icall_void_RID_RID_RID(___mb.mb_instance_set_use_lightmap, (const Object *) this, instance, lightmap_instance, lightmap);
}

void VisualServer::instance_set_visible(const RID instance, const bool visible) {
	___godot_icall_void_RID_bool(___mb.mb_instance_set_visible, (const Object *) this, instance, visible);
}

Array VisualServer::instances_cull_aabb(const AABB aabb, const RID scenario) const {
	return ___godot_icall_Array_AABB_RID(___mb.mb_instances_cull_aabb, (const Object *) this, aabb, scenario);
}

Array VisualServer::instances_cull_convex(const Array convex, const RID scenario) const {
	return ___godot_icall_Array_Array_RID(___mb.mb_instances_cull_convex, (const Object *) this, convex, scenario);
}

Array VisualServer::instances_cull_ray(const Vector3 from, const Vector3 to, const RID scenario) const {
	return ___godot_icall_Array_Vector3_Vector3_RID(___mb.mb_instances_cull_ray, (const Object *) this, from, to, scenario);
}

void VisualServer::light_directional_set_blend_splits(const RID light, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_light_directional_set_blend_splits, (const Object *) this, light, enable);
}

void VisualServer::light_directional_set_shadow_depth_range_mode(const RID light, const int64_t range_mode) {
	___godot_icall_void_RID_int(___mb.mb_light_directional_set_shadow_depth_range_mode, (const Object *) this, light, range_mode);
}

void VisualServer::light_directional_set_shadow_mode(const RID light, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_light_directional_set_shadow_mode, (const Object *) this, light, mode);
}

void VisualServer::light_omni_set_shadow_detail(const RID light, const int64_t detail) {
	___godot_icall_void_RID_int(___mb.mb_light_omni_set_shadow_detail, (const Object *) this, light, detail);
}

void VisualServer::light_omni_set_shadow_mode(const RID light, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_light_omni_set_shadow_mode, (const Object *) this, light, mode);
}

void VisualServer::light_set_color(const RID light, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_light_set_color, (const Object *) this, light, color);
}

void VisualServer::light_set_cull_mask(const RID light, const int64_t mask) {
	___godot_icall_void_RID_int(___mb.mb_light_set_cull_mask, (const Object *) this, light, mask);
}

void VisualServer::light_set_negative(const RID light, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_light_set_negative, (const Object *) this, light, enable);
}

void VisualServer::light_set_param(const RID light, const int64_t param, const real_t value) {
	___godot_icall_void_RID_int_float(___mb.mb_light_set_param, (const Object *) this, light, param, value);
}

void VisualServer::light_set_projector(const RID light, const RID texture) {
	___godot_icall_void_RID_RID(___mb.mb_light_set_projector, (const Object *) this, light, texture);
}

void VisualServer::light_set_reverse_cull_face_mode(const RID light, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_light_set_reverse_cull_face_mode, (const Object *) this, light, enabled);
}

void VisualServer::light_set_shadow(const RID light, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_light_set_shadow, (const Object *) this, light, enabled);
}

void VisualServer::light_set_shadow_color(const RID light, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_light_set_shadow_color, (const Object *) this, light, color);
}

void VisualServer::light_set_use_gi(const RID light, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_light_set_use_gi, (const Object *) this, light, enabled);
}

RID VisualServer::lightmap_capture_create() {
	return ___godot_icall_RID(___mb.mb_lightmap_capture_create, (const Object *) this);
}

AABB VisualServer::lightmap_capture_get_bounds(const RID capture) const {
	return ___godot_icall_AABB_RID(___mb.mb_lightmap_capture_get_bounds, (const Object *) this, capture);
}

real_t VisualServer::lightmap_capture_get_energy(const RID capture) const {
	return ___godot_icall_float_RID(___mb.mb_lightmap_capture_get_energy, (const Object *) this, capture);
}

PoolByteArray VisualServer::lightmap_capture_get_octree(const RID capture) const {
	return ___godot_icall_PoolByteArray_RID(___mb.mb_lightmap_capture_get_octree, (const Object *) this, capture);
}

int64_t VisualServer::lightmap_capture_get_octree_cell_subdiv(const RID capture) const {
	return ___godot_icall_int_RID(___mb.mb_lightmap_capture_get_octree_cell_subdiv, (const Object *) this, capture);
}

Transform VisualServer::lightmap_capture_get_octree_cell_transform(const RID capture) const {
	return ___godot_icall_Transform_RID(___mb.mb_lightmap_capture_get_octree_cell_transform, (const Object *) this, capture);
}

void VisualServer::lightmap_capture_set_bounds(const RID capture, const AABB bounds) {
	___godot_icall_void_RID_AABB(___mb.mb_lightmap_capture_set_bounds, (const Object *) this, capture, bounds);
}

void VisualServer::lightmap_capture_set_energy(const RID capture, const real_t energy) {
	___godot_icall_void_RID_float(___mb.mb_lightmap_capture_set_energy, (const Object *) this, capture, energy);
}

void VisualServer::lightmap_capture_set_octree(const RID capture, const PoolByteArray octree) {
	___godot_icall_void_RID_PoolByteArray(___mb.mb_lightmap_capture_set_octree, (const Object *) this, capture, octree);
}

void VisualServer::lightmap_capture_set_octree_cell_subdiv(const RID capture, const int64_t subdiv) {
	___godot_icall_void_RID_int(___mb.mb_lightmap_capture_set_octree_cell_subdiv, (const Object *) this, capture, subdiv);
}

void VisualServer::lightmap_capture_set_octree_cell_transform(const RID capture, const Transform xform) {
	___godot_icall_void_RID_Transform(___mb.mb_lightmap_capture_set_octree_cell_transform, (const Object *) this, capture, xform);
}

RID VisualServer::make_sphere_mesh(const int64_t latitudes, const int64_t longitudes, const real_t radius) {
	return ___godot_icall_RID_int_int_float(___mb.mb_make_sphere_mesh, (const Object *) this, latitudes, longitudes, radius);
}

RID VisualServer::material_create() {
	return ___godot_icall_RID(___mb.mb_material_create, (const Object *) this);
}

Variant VisualServer::material_get_param(const RID material, const String parameter) const {
	return ___godot_icall_Variant_RID_String(___mb.mb_material_get_param, (const Object *) this, material, parameter);
}

Variant VisualServer::material_get_param_default(const RID material, const String parameter) const {
	return ___godot_icall_Variant_RID_String(___mb.mb_material_get_param_default, (const Object *) this, material, parameter);
}

RID VisualServer::material_get_shader(const RID shader_material) const {
	return ___godot_icall_RID_RID(___mb.mb_material_get_shader, (const Object *) this, shader_material);
}

void VisualServer::material_set_line_width(const RID material, const real_t width) {
	___godot_icall_void_RID_float(___mb.mb_material_set_line_width, (const Object *) this, material, width);
}

void VisualServer::material_set_next_pass(const RID material, const RID next_material) {
	___godot_icall_void_RID_RID(___mb.mb_material_set_next_pass, (const Object *) this, material, next_material);
}

void VisualServer::material_set_param(const RID material, const String parameter, const Variant value) {
	___godot_icall_void_RID_String_Variant(___mb.mb_material_set_param, (const Object *) this, material, parameter, value);
}

void VisualServer::material_set_render_priority(const RID material, const int64_t priority) {
	___godot_icall_void_RID_int(___mb.mb_material_set_render_priority, (const Object *) this, material, priority);
}

void VisualServer::material_set_shader(const RID shader_material, const RID shader) {
	___godot_icall_void_RID_RID(___mb.mb_material_set_shader, (const Object *) this, shader_material, shader);
}

void VisualServer::mesh_add_surface_from_arrays(const RID mesh, const int64_t primitive, const Array arrays, const Array blend_shapes, const int64_t compress_format) {
	___godot_icall_void_RID_int_Array_Array_int(___mb.mb_mesh_add_surface_from_arrays, (const Object *) this, mesh, primitive, arrays, blend_shapes, compress_format);
}

void VisualServer::mesh_clear(const RID mesh) {
	___godot_icall_void_RID(___mb.mb_mesh_clear, (const Object *) this, mesh);
}

RID VisualServer::mesh_create() {
	return ___godot_icall_RID(___mb.mb_mesh_create, (const Object *) this);
}

int64_t VisualServer::mesh_get_blend_shape_count(const RID mesh) const {
	return ___godot_icall_int_RID(___mb.mb_mesh_get_blend_shape_count, (const Object *) this, mesh);
}

VisualServer::BlendShapeMode VisualServer::mesh_get_blend_shape_mode(const RID mesh) const {
	return (VisualServer::BlendShapeMode) ___godot_icall_int_RID(___mb.mb_mesh_get_blend_shape_mode, (const Object *) this, mesh);
}

AABB VisualServer::mesh_get_custom_aabb(const RID mesh) const {
	return ___godot_icall_AABB_RID(___mb.mb_mesh_get_custom_aabb, (const Object *) this, mesh);
}

int64_t VisualServer::mesh_get_surface_count(const RID mesh) const {
	return ___godot_icall_int_RID(___mb.mb_mesh_get_surface_count, (const Object *) this, mesh);
}

void VisualServer::mesh_remove_surface(const RID mesh, const int64_t index) {
	___godot_icall_void_RID_int(___mb.mb_mesh_remove_surface, (const Object *) this, mesh, index);
}

void VisualServer::mesh_set_blend_shape_count(const RID mesh, const int64_t amount) {
	___godot_icall_void_RID_int(___mb.mb_mesh_set_blend_shape_count, (const Object *) this, mesh, amount);
}

void VisualServer::mesh_set_blend_shape_mode(const RID mesh, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_mesh_set_blend_shape_mode, (const Object *) this, mesh, mode);
}

void VisualServer::mesh_set_custom_aabb(const RID mesh, const AABB aabb) {
	___godot_icall_void_RID_AABB(___mb.mb_mesh_set_custom_aabb, (const Object *) this, mesh, aabb);
}

AABB VisualServer::mesh_surface_get_aabb(const RID mesh, const int64_t surface) const {
	return ___godot_icall_AABB_RID_int(___mb.mb_mesh_surface_get_aabb, (const Object *) this, mesh, surface);
}

PoolByteArray VisualServer::mesh_surface_get_array(const RID mesh, const int64_t surface) const {
	return ___godot_icall_PoolByteArray_RID_int(___mb.mb_mesh_surface_get_array, (const Object *) this, mesh, surface);
}

int64_t VisualServer::mesh_surface_get_array_index_len(const RID mesh, const int64_t surface) const {
	return ___godot_icall_int_RID_int(___mb.mb_mesh_surface_get_array_index_len, (const Object *) this, mesh, surface);
}

int64_t VisualServer::mesh_surface_get_array_len(const RID mesh, const int64_t surface) const {
	return ___godot_icall_int_RID_int(___mb.mb_mesh_surface_get_array_len, (const Object *) this, mesh, surface);
}

Array VisualServer::mesh_surface_get_arrays(const RID mesh, const int64_t surface) const {
	return ___godot_icall_Array_RID_int(___mb.mb_mesh_surface_get_arrays, (const Object *) this, mesh, surface);
}

Array VisualServer::mesh_surface_get_blend_shape_arrays(const RID mesh, const int64_t surface) const {
	return ___godot_icall_Array_RID_int(___mb.mb_mesh_surface_get_blend_shape_arrays, (const Object *) this, mesh, surface);
}

int64_t VisualServer::mesh_surface_get_format(const RID mesh, const int64_t surface) const {
	return ___godot_icall_int_RID_int(___mb.mb_mesh_surface_get_format, (const Object *) this, mesh, surface);
}

int64_t VisualServer::mesh_surface_get_format_offset(const int64_t format, const int64_t vertex_len, const int64_t index_len, const int64_t array_index) const {
	return ___godot_icall_int_int_int_int_int(___mb.mb_mesh_surface_get_format_offset, (const Object *) this, format, vertex_len, index_len, array_index);
}

int64_t VisualServer::mesh_surface_get_format_stride(const int64_t format, const int64_t vertex_len, const int64_t index_len) const {
	return ___godot_icall_int_int_int_int(___mb.mb_mesh_surface_get_format_stride, (const Object *) this, format, vertex_len, index_len);
}

PoolByteArray VisualServer::mesh_surface_get_index_array(const RID mesh, const int64_t surface) const {
	return ___godot_icall_PoolByteArray_RID_int(___mb.mb_mesh_surface_get_index_array, (const Object *) this, mesh, surface);
}

RID VisualServer::mesh_surface_get_material(const RID mesh, const int64_t surface) const {
	return ___godot_icall_RID_RID_int(___mb.mb_mesh_surface_get_material, (const Object *) this, mesh, surface);
}

VisualServer::PrimitiveType VisualServer::mesh_surface_get_primitive_type(const RID mesh, const int64_t surface) const {
	return (VisualServer::PrimitiveType) ___godot_icall_int_RID_int(___mb.mb_mesh_surface_get_primitive_type, (const Object *) this, mesh, surface);
}

Array VisualServer::mesh_surface_get_skeleton_aabb(const RID mesh, const int64_t surface) const {
	return ___godot_icall_Array_RID_int(___mb.mb_mesh_surface_get_skeleton_aabb, (const Object *) this, mesh, surface);
}

void VisualServer::mesh_surface_set_material(const RID mesh, const int64_t surface, const RID material) {
	___godot_icall_void_RID_int_RID(___mb.mb_mesh_surface_set_material, (const Object *) this, mesh, surface, material);
}

void VisualServer::mesh_surface_update_region(const RID mesh, const int64_t surface, const int64_t offset, const PoolByteArray data) {
	___godot_icall_void_RID_int_int_PoolByteArray(___mb.mb_mesh_surface_update_region, (const Object *) this, mesh, surface, offset, data);
}

void VisualServer::multimesh_allocate(const RID multimesh, const int64_t instances, const int64_t transform_format, const int64_t color_format, const int64_t custom_data_format) {
	___godot_icall_void_RID_int_int_int_int(___mb.mb_multimesh_allocate, (const Object *) this, multimesh, instances, transform_format, color_format, custom_data_format);
}

RID VisualServer::multimesh_create() {
	return ___godot_icall_RID(___mb.mb_multimesh_create, (const Object *) this);
}

AABB VisualServer::multimesh_get_aabb(const RID multimesh) const {
	return ___godot_icall_AABB_RID(___mb.mb_multimesh_get_aabb, (const Object *) this, multimesh);
}

int64_t VisualServer::multimesh_get_instance_count(const RID multimesh) const {
	return ___godot_icall_int_RID(___mb.mb_multimesh_get_instance_count, (const Object *) this, multimesh);
}

RID VisualServer::multimesh_get_mesh(const RID multimesh) const {
	return ___godot_icall_RID_RID(___mb.mb_multimesh_get_mesh, (const Object *) this, multimesh);
}

int64_t VisualServer::multimesh_get_visible_instances(const RID multimesh) const {
	return ___godot_icall_int_RID(___mb.mb_multimesh_get_visible_instances, (const Object *) this, multimesh);
}

Color VisualServer::multimesh_instance_get_color(const RID multimesh, const int64_t index) const {
	return ___godot_icall_Color_RID_int(___mb.mb_multimesh_instance_get_color, (const Object *) this, multimesh, index);
}

Color VisualServer::multimesh_instance_get_custom_data(const RID multimesh, const int64_t index) const {
	return ___godot_icall_Color_RID_int(___mb.mb_multimesh_instance_get_custom_data, (const Object *) this, multimesh, index);
}

Transform VisualServer::multimesh_instance_get_transform(const RID multimesh, const int64_t index) const {
	return ___godot_icall_Transform_RID_int(___mb.mb_multimesh_instance_get_transform, (const Object *) this, multimesh, index);
}

Transform2D VisualServer::multimesh_instance_get_transform_2d(const RID multimesh, const int64_t index) const {
	return ___godot_icall_Transform2D_RID_int(___mb.mb_multimesh_instance_get_transform_2d, (const Object *) this, multimesh, index);
}

void VisualServer::multimesh_instance_set_color(const RID multimesh, const int64_t index, const Color color) {
	___godot_icall_void_RID_int_Color(___mb.mb_multimesh_instance_set_color, (const Object *) this, multimesh, index, color);
}

void VisualServer::multimesh_instance_set_custom_data(const RID multimesh, const int64_t index, const Color custom_data) {
	___godot_icall_void_RID_int_Color(___mb.mb_multimesh_instance_set_custom_data, (const Object *) this, multimesh, index, custom_data);
}

void VisualServer::multimesh_instance_set_transform(const RID multimesh, const int64_t index, const Transform transform) {
	___godot_icall_void_RID_int_Transform(___mb.mb_multimesh_instance_set_transform, (const Object *) this, multimesh, index, transform);
}

void VisualServer::multimesh_instance_set_transform_2d(const RID multimesh, const int64_t index, const Transform2D transform) {
	___godot_icall_void_RID_int_Transform2D(___mb.mb_multimesh_instance_set_transform_2d, (const Object *) this, multimesh, index, transform);
}

void VisualServer::multimesh_set_as_bulk_array(const RID multimesh, const PoolRealArray array) {
	___godot_icall_void_RID_PoolRealArray(___mb.mb_multimesh_set_as_bulk_array, (const Object *) this, multimesh, array);
}

void VisualServer::multimesh_set_mesh(const RID multimesh, const RID mesh) {
	___godot_icall_void_RID_RID(___mb.mb_multimesh_set_mesh, (const Object *) this, multimesh, mesh);
}

void VisualServer::multimesh_set_visible_instances(const RID multimesh, const int64_t visible) {
	___godot_icall_void_RID_int(___mb.mb_multimesh_set_visible_instances, (const Object *) this, multimesh, visible);
}

RID VisualServer::omni_light_create() {
	return ___godot_icall_RID(___mb.mb_omni_light_create, (const Object *) this);
}

RID VisualServer::particles_create() {
	return ___godot_icall_RID(___mb.mb_particles_create, (const Object *) this);
}

AABB VisualServer::particles_get_current_aabb(const RID particles) {
	return ___godot_icall_AABB_RID(___mb.mb_particles_get_current_aabb, (const Object *) this, particles);
}

bool VisualServer::particles_get_emitting(const RID particles) {
	return ___godot_icall_bool_RID(___mb.mb_particles_get_emitting, (const Object *) this, particles);
}

bool VisualServer::particles_is_inactive(const RID particles) {
	return ___godot_icall_bool_RID(___mb.mb_particles_is_inactive, (const Object *) this, particles);
}

void VisualServer::particles_request_process(const RID particles) {
	___godot_icall_void_RID(___mb.mb_particles_request_process, (const Object *) this, particles);
}

void VisualServer::particles_restart(const RID particles) {
	___godot_icall_void_RID(___mb.mb_particles_restart, (const Object *) this, particles);
}

void VisualServer::particles_set_amount(const RID particles, const int64_t amount) {
	___godot_icall_void_RID_int(___mb.mb_particles_set_amount, (const Object *) this, particles, amount);
}

void VisualServer::particles_set_custom_aabb(const RID particles, const AABB aabb) {
	___godot_icall_void_RID_AABB(___mb.mb_particles_set_custom_aabb, (const Object *) this, particles, aabb);
}

void VisualServer::particles_set_draw_order(const RID particles, const int64_t order) {
	___godot_icall_void_RID_int(___mb.mb_particles_set_draw_order, (const Object *) this, particles, order);
}

void VisualServer::particles_set_draw_pass_mesh(const RID particles, const int64_t pass, const RID mesh) {
	___godot_icall_void_RID_int_RID(___mb.mb_particles_set_draw_pass_mesh, (const Object *) this, particles, pass, mesh);
}

void VisualServer::particles_set_draw_passes(const RID particles, const int64_t count) {
	___godot_icall_void_RID_int(___mb.mb_particles_set_draw_passes, (const Object *) this, particles, count);
}

void VisualServer::particles_set_emission_transform(const RID particles, const Transform transform) {
	___godot_icall_void_RID_Transform(___mb.mb_particles_set_emission_transform, (const Object *) this, particles, transform);
}

void VisualServer::particles_set_emitting(const RID particles, const bool emitting) {
	___godot_icall_void_RID_bool(___mb.mb_particles_set_emitting, (const Object *) this, particles, emitting);
}

void VisualServer::particles_set_explosiveness_ratio(const RID particles, const real_t ratio) {
	___godot_icall_void_RID_float(___mb.mb_particles_set_explosiveness_ratio, (const Object *) this, particles, ratio);
}

void VisualServer::particles_set_fixed_fps(const RID particles, const int64_t fps) {
	___godot_icall_void_RID_int(___mb.mb_particles_set_fixed_fps, (const Object *) this, particles, fps);
}

void VisualServer::particles_set_fractional_delta(const RID particles, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_particles_set_fractional_delta, (const Object *) this, particles, enable);
}

void VisualServer::particles_set_lifetime(const RID particles, const real_t lifetime) {
	___godot_icall_void_RID_float(___mb.mb_particles_set_lifetime, (const Object *) this, particles, lifetime);
}

void VisualServer::particles_set_one_shot(const RID particles, const bool one_shot) {
	___godot_icall_void_RID_bool(___mb.mb_particles_set_one_shot, (const Object *) this, particles, one_shot);
}

void VisualServer::particles_set_pre_process_time(const RID particles, const real_t time) {
	___godot_icall_void_RID_float(___mb.mb_particles_set_pre_process_time, (const Object *) this, particles, time);
}

void VisualServer::particles_set_process_material(const RID particles, const RID material) {
	___godot_icall_void_RID_RID(___mb.mb_particles_set_process_material, (const Object *) this, particles, material);
}

void VisualServer::particles_set_randomness_ratio(const RID particles, const real_t ratio) {
	___godot_icall_void_RID_float(___mb.mb_particles_set_randomness_ratio, (const Object *) this, particles, ratio);
}

void VisualServer::particles_set_speed_scale(const RID particles, const real_t scale) {
	___godot_icall_void_RID_float(___mb.mb_particles_set_speed_scale, (const Object *) this, particles, scale);
}

void VisualServer::particles_set_use_local_coordinates(const RID particles, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_particles_set_use_local_coordinates, (const Object *) this, particles, enable);
}

RID VisualServer::reflection_probe_create() {
	return ___godot_icall_RID(___mb.mb_reflection_probe_create, (const Object *) this);
}

void VisualServer::reflection_probe_set_as_interior(const RID probe, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_reflection_probe_set_as_interior, (const Object *) this, probe, enable);
}

void VisualServer::reflection_probe_set_cull_mask(const RID probe, const int64_t layers) {
	___godot_icall_void_RID_int(___mb.mb_reflection_probe_set_cull_mask, (const Object *) this, probe, layers);
}

void VisualServer::reflection_probe_set_enable_box_projection(const RID probe, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_reflection_probe_set_enable_box_projection, (const Object *) this, probe, enable);
}

void VisualServer::reflection_probe_set_enable_shadows(const RID probe, const bool enable) {
	___godot_icall_void_RID_bool(___mb.mb_reflection_probe_set_enable_shadows, (const Object *) this, probe, enable);
}

void VisualServer::reflection_probe_set_extents(const RID probe, const Vector3 extents) {
	___godot_icall_void_RID_Vector3(___mb.mb_reflection_probe_set_extents, (const Object *) this, probe, extents);
}

void VisualServer::reflection_probe_set_intensity(const RID probe, const real_t intensity) {
	___godot_icall_void_RID_float(___mb.mb_reflection_probe_set_intensity, (const Object *) this, probe, intensity);
}

void VisualServer::reflection_probe_set_interior_ambient(const RID probe, const Color color) {
	___godot_icall_void_RID_Color(___mb.mb_reflection_probe_set_interior_ambient, (const Object *) this, probe, color);
}

void VisualServer::reflection_probe_set_interior_ambient_energy(const RID probe, const real_t energy) {
	___godot_icall_void_RID_float(___mb.mb_reflection_probe_set_interior_ambient_energy, (const Object *) this, probe, energy);
}

void VisualServer::reflection_probe_set_interior_ambient_probe_contribution(const RID probe, const real_t contrib) {
	___godot_icall_void_RID_float(___mb.mb_reflection_probe_set_interior_ambient_probe_contribution, (const Object *) this, probe, contrib);
}

void VisualServer::reflection_probe_set_max_distance(const RID probe, const real_t distance) {
	___godot_icall_void_RID_float(___mb.mb_reflection_probe_set_max_distance, (const Object *) this, probe, distance);
}

void VisualServer::reflection_probe_set_origin_offset(const RID probe, const Vector3 offset) {
	___godot_icall_void_RID_Vector3(___mb.mb_reflection_probe_set_origin_offset, (const Object *) this, probe, offset);
}

void VisualServer::reflection_probe_set_update_mode(const RID probe, const int64_t mode) {
	___godot_icall_void_RID_int(___mb.mb_reflection_probe_set_update_mode, (const Object *) this, probe, mode);
}

void VisualServer::request_frame_drawn_callback(const Object *where, const String method, const Variant userdata) {
	___godot_icall_void_Object_String_Variant(___mb.mb_request_frame_drawn_callback, (const Object *) this, where, method, userdata);
}

RID VisualServer::scenario_create() {
	return ___godot_icall_RID(___mb.mb_scenario_create, (const Object *) this);
}

void VisualServer::scenario_set_debug(const RID scenario, const int64_t debug_mode) {
	___godot_icall_void_RID_int(___mb.mb_scenario_set_debug, (const Object *) this, scenario, debug_mode);
}

void VisualServer::scenario_set_environment(const RID scenario, const RID environment) {
	___godot_icall_void_RID_RID(___mb.mb_scenario_set_environment, (const Object *) this, scenario, environment);
}

void VisualServer::scenario_set_fallback_environment(const RID scenario, const RID environment) {
	___godot_icall_void_RID_RID(___mb.mb_scenario_set_fallback_environment, (const Object *) this, scenario, environment);
}

void VisualServer::scenario_set_reflection_atlas_size(const RID scenario, const int64_t size, const int64_t subdiv) {
	___godot_icall_void_RID_int_int(___mb.mb_scenario_set_reflection_atlas_size, (const Object *) this, scenario, size, subdiv);
}

void VisualServer::set_boot_image(const Ref<Image> image, const Color color, const bool scale, const bool use_filter) {
	___godot_icall_void_Object_Color_bool_bool(___mb.mb_set_boot_image, (const Object *) this, image.ptr(), color, scale, use_filter);
}

void VisualServer::set_debug_generate_wireframes(const bool generate) {
	___godot_icall_void_bool(___mb.mb_set_debug_generate_wireframes, (const Object *) this, generate);
}

void VisualServer::set_default_clear_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_default_clear_color, (const Object *) this, color);
}

RID VisualServer::shader_create() {
	return ___godot_icall_RID(___mb.mb_shader_create, (const Object *) this);
}

String VisualServer::shader_get_code(const RID shader) const {
	return ___godot_icall_String_RID(___mb.mb_shader_get_code, (const Object *) this, shader);
}

RID VisualServer::shader_get_default_texture_param(const RID shader, const String name) const {
	return ___godot_icall_RID_RID_String(___mb.mb_shader_get_default_texture_param, (const Object *) this, shader, name);
}

Array VisualServer::shader_get_param_list(const RID shader) const {
	return ___godot_icall_Array_RID(___mb.mb_shader_get_param_list, (const Object *) this, shader);
}

void VisualServer::shader_set_code(const RID shader, const String code) {
	___godot_icall_void_RID_String(___mb.mb_shader_set_code, (const Object *) this, shader, code);
}

void VisualServer::shader_set_default_texture_param(const RID shader, const String name, const RID texture) {
	___godot_icall_void_RID_String_RID(___mb.mb_shader_set_default_texture_param, (const Object *) this, shader, name, texture);
}

void VisualServer::skeleton_allocate(const RID skeleton, const int64_t bones, const bool is_2d_skeleton) {
	___godot_icall_void_RID_int_bool(___mb.mb_skeleton_allocate, (const Object *) this, skeleton, bones, is_2d_skeleton);
}

Transform VisualServer::skeleton_bone_get_transform(const RID skeleton, const int64_t bone) const {
	return ___godot_icall_Transform_RID_int(___mb.mb_skeleton_bone_get_transform, (const Object *) this, skeleton, bone);
}

Transform2D VisualServer::skeleton_bone_get_transform_2d(const RID skeleton, const int64_t bone) const {
	return ___godot_icall_Transform2D_RID_int(___mb.mb_skeleton_bone_get_transform_2d, (const Object *) this, skeleton, bone);
}

void VisualServer::skeleton_bone_set_transform(const RID skeleton, const int64_t bone, const Transform transform) {
	___godot_icall_void_RID_int_Transform(___mb.mb_skeleton_bone_set_transform, (const Object *) this, skeleton, bone, transform);
}

void VisualServer::skeleton_bone_set_transform_2d(const RID skeleton, const int64_t bone, const Transform2D transform) {
	___godot_icall_void_RID_int_Transform2D(___mb.mb_skeleton_bone_set_transform_2d, (const Object *) this, skeleton, bone, transform);
}

RID VisualServer::skeleton_create() {
	return ___godot_icall_RID(___mb.mb_skeleton_create, (const Object *) this);
}

int64_t VisualServer::skeleton_get_bone_count(const RID skeleton) const {
	return ___godot_icall_int_RID(___mb.mb_skeleton_get_bone_count, (const Object *) this, skeleton);
}

RID VisualServer::sky_create() {
	return ___godot_icall_RID(___mb.mb_sky_create, (const Object *) this);
}

void VisualServer::sky_set_texture(const RID sky, const RID cube_map, const int64_t radiance_size) {
	___godot_icall_void_RID_RID_int(___mb.mb_sky_set_texture, (const Object *) this, sky, cube_map, radiance_size);
}

RID VisualServer::spot_light_create() {
	return ___godot_icall_RID(___mb.mb_spot_light_create, (const Object *) this);
}

void VisualServer::sync() {
	___godot_icall_void(___mb.mb_sync, (const Object *) this);
}

void VisualServer::texture_allocate(const RID texture, const int64_t width, const int64_t height, const int64_t depth_3d, const int64_t format, const int64_t type, const int64_t flags) {
	___godot_icall_void_RID_int_int_int_int_int_int(___mb.mb_texture_allocate, (const Object *) this, texture, width, height, depth_3d, format, type, flags);
}

void VisualServer::texture_bind(const RID texture, const int64_t number) {
	___godot_icall_void_RID_int(___mb.mb_texture_bind, (const Object *) this, texture, number);
}

RID VisualServer::texture_create() {
	return ___godot_icall_RID(___mb.mb_texture_create, (const Object *) this);
}

RID VisualServer::texture_create_from_image(const Ref<Image> image, const int64_t flags) {
	return ___godot_icall_RID_Object_int(___mb.mb_texture_create_from_image, (const Object *) this, image.ptr(), flags);
}

Array VisualServer::texture_debug_usage() {
	return ___godot_icall_Array(___mb.mb_texture_debug_usage, (const Object *) this);
}

Ref<Image> VisualServer::texture_get_data(const RID texture, const int64_t cube_side) const {
	return Ref<Image>::__internal_constructor(___godot_icall_Object_RID_int(___mb.mb_texture_get_data, (const Object *) this, texture, cube_side));
}

int64_t VisualServer::texture_get_depth(const RID texture) const {
	return ___godot_icall_int_RID(___mb.mb_texture_get_depth, (const Object *) this, texture);
}

int64_t VisualServer::texture_get_flags(const RID texture) const {
	return ___godot_icall_int_RID(___mb.mb_texture_get_flags, (const Object *) this, texture);
}

Image::Format VisualServer::texture_get_format(const RID texture) const {
	return (Image::Format) ___godot_icall_int_RID(___mb.mb_texture_get_format, (const Object *) this, texture);
}

int64_t VisualServer::texture_get_height(const RID texture) const {
	return ___godot_icall_int_RID(___mb.mb_texture_get_height, (const Object *) this, texture);
}

String VisualServer::texture_get_path(const RID texture) const {
	return ___godot_icall_String_RID(___mb.mb_texture_get_path, (const Object *) this, texture);
}

int64_t VisualServer::texture_get_texid(const RID texture) const {
	return ___godot_icall_int_RID(___mb.mb_texture_get_texid, (const Object *) this, texture);
}

VisualServer::TextureType VisualServer::texture_get_type(const RID texture) const {
	return (VisualServer::TextureType) ___godot_icall_int_RID(___mb.mb_texture_get_type, (const Object *) this, texture);
}

int64_t VisualServer::texture_get_width(const RID texture) const {
	return ___godot_icall_int_RID(___mb.mb_texture_get_width, (const Object *) this, texture);
}

void VisualServer::texture_set_data(const RID texture, const Ref<Image> image, const int64_t layer) {
	___godot_icall_void_RID_Object_int(___mb.mb_texture_set_data, (const Object *) this, texture, image.ptr(), layer);
}

void VisualServer::texture_set_data_partial(const RID texture, const Ref<Image> image, const int64_t src_x, const int64_t src_y, const int64_t src_w, const int64_t src_h, const int64_t dst_x, const int64_t dst_y, const int64_t dst_mip, const int64_t layer) {
	___godot_icall_void_RID_Object_int_int_int_int_int_int_int_int(___mb.mb_texture_set_data_partial, (const Object *) this, texture, image.ptr(), src_x, src_y, src_w, src_h, dst_x, dst_y, dst_mip, layer);
}

void VisualServer::texture_set_flags(const RID texture, const int64_t flags) {
	___godot_icall_void_RID_int(___mb.mb_texture_set_flags, (const Object *) this, texture, flags);
}

void VisualServer::texture_set_path(const RID texture, const String path) {
	___godot_icall_void_RID_String(___mb.mb_texture_set_path, (const Object *) this, texture, path);
}

void VisualServer::texture_set_shrink_all_x2_on_set_data(const bool shrink) {
	___godot_icall_void_bool(___mb.mb_texture_set_shrink_all_x2_on_set_data, (const Object *) this, shrink);
}

void VisualServer::texture_set_size_override(const RID texture, const int64_t width, const int64_t height, const int64_t depth) {
	___godot_icall_void_RID_int_int_int(___mb.mb_texture_set_size_override, (const Object *) this, texture, width, height, depth);
}

void VisualServer::textures_keep_original(const bool enable) {
	___godot_icall_void_bool(___mb.mb_textures_keep_original, (const Object *) this, enable);
}

void VisualServer::viewport_attach_camera(const RID viewport, const RID camera) {
	___godot_icall_void_RID_RID(___mb.mb_viewport_attach_camera, (const Object *) this, viewport, camera);
}

void VisualServer::viewport_attach_canvas(const RID viewport, const RID canvas) {
	___godot_icall_void_RID_RID(___mb.mb_viewport_attach_canvas, (const Object *) this, viewport, canvas);
}

void VisualServer::viewport_attach_to_screen(const RID viewport, const Rect2 rect, const int64_t screen) {
	___godot_icall_void_RID_Rect2_int(___mb.mb_viewport_attach_to_screen, (const Object *) this, viewport, rect, screen);
}

RID VisualServer::viewport_create() {
	return ___godot_icall_RID(___mb.mb_viewport_create, (const Object *) this);
}

void VisualServer::viewport_detach(const RID viewport) {
	___godot_icall_void_RID(___mb.mb_viewport_detach, (const Object *) this, viewport);
}

int64_t VisualServer::viewport_get_render_info(const RID viewport, const int64_t info) {
	return ___godot_icall_int_RID_int(___mb.mb_viewport_get_render_info, (const Object *) this, viewport, info);
}

RID VisualServer::viewport_get_texture(const RID viewport) const {
	return ___godot_icall_RID_RID(___mb.mb_viewport_get_texture, (const Object *) this, viewport);
}

void VisualServer::viewport_remove_canvas(const RID viewport, const RID canvas) {
	___godot_icall_void_RID_RID(___mb.mb_viewport_remove_canvas, (const Object *) this, viewport, canvas);
}

void VisualServer::viewport_set_active(const RID viewport, const bool active) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_active, (const Object *) this, viewport, active);
}

void VisualServer::viewport_set_canvas_stacking(const RID viewport, const RID canvas, const int64_t layer, const int64_t sublayer) {
	___godot_icall_void_RID_RID_int_int(___mb.mb_viewport_set_canvas_stacking, (const Object *) this, viewport, canvas, layer, sublayer);
}

void VisualServer::viewport_set_canvas_transform(const RID viewport, const RID canvas, const Transform2D offset) {
	___godot_icall_void_RID_RID_Transform2D(___mb.mb_viewport_set_canvas_transform, (const Object *) this, viewport, canvas, offset);
}

void VisualServer::viewport_set_clear_mode(const RID viewport, const int64_t clear_mode) {
	___godot_icall_void_RID_int(___mb.mb_viewport_set_clear_mode, (const Object *) this, viewport, clear_mode);
}

void VisualServer::viewport_set_debug_draw(const RID viewport, const int64_t draw) {
	___godot_icall_void_RID_int(___mb.mb_viewport_set_debug_draw, (const Object *) this, viewport, draw);
}

void VisualServer::viewport_set_disable_3d(const RID viewport, const bool disabled) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_disable_3d, (const Object *) this, viewport, disabled);
}

void VisualServer::viewport_set_disable_environment(const RID viewport, const bool disabled) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_disable_environment, (const Object *) this, viewport, disabled);
}

void VisualServer::viewport_set_global_canvas_transform(const RID viewport, const Transform2D transform) {
	___godot_icall_void_RID_Transform2D(___mb.mb_viewport_set_global_canvas_transform, (const Object *) this, viewport, transform);
}

void VisualServer::viewport_set_hdr(const RID viewport, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_hdr, (const Object *) this, viewport, enabled);
}

void VisualServer::viewport_set_hide_canvas(const RID viewport, const bool hidden) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_hide_canvas, (const Object *) this, viewport, hidden);
}

void VisualServer::viewport_set_hide_scenario(const RID viewport, const bool hidden) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_hide_scenario, (const Object *) this, viewport, hidden);
}

void VisualServer::viewport_set_msaa(const RID viewport, const int64_t msaa) {
	___godot_icall_void_RID_int(___mb.mb_viewport_set_msaa, (const Object *) this, viewport, msaa);
}

void VisualServer::viewport_set_parent_viewport(const RID viewport, const RID parent_viewport) {
	___godot_icall_void_RID_RID(___mb.mb_viewport_set_parent_viewport, (const Object *) this, viewport, parent_viewport);
}

void VisualServer::viewport_set_render_direct_to_screen(const RID viewport, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_render_direct_to_screen, (const Object *) this, viewport, enabled);
}

void VisualServer::viewport_set_scenario(const RID viewport, const RID scenario) {
	___godot_icall_void_RID_RID(___mb.mb_viewport_set_scenario, (const Object *) this, viewport, scenario);
}

void VisualServer::viewport_set_shadow_atlas_quadrant_subdivision(const RID viewport, const int64_t quadrant, const int64_t subdivision) {
	___godot_icall_void_RID_int_int(___mb.mb_viewport_set_shadow_atlas_quadrant_subdivision, (const Object *) this, viewport, quadrant, subdivision);
}

void VisualServer::viewport_set_shadow_atlas_size(const RID viewport, const int64_t size) {
	___godot_icall_void_RID_int(___mb.mb_viewport_set_shadow_atlas_size, (const Object *) this, viewport, size);
}

void VisualServer::viewport_set_size(const RID viewport, const int64_t width, const int64_t height) {
	___godot_icall_void_RID_int_int(___mb.mb_viewport_set_size, (const Object *) this, viewport, width, height);
}

void VisualServer::viewport_set_transparent_background(const RID viewport, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_transparent_background, (const Object *) this, viewport, enabled);
}

void VisualServer::viewport_set_update_mode(const RID viewport, const int64_t update_mode) {
	___godot_icall_void_RID_int(___mb.mb_viewport_set_update_mode, (const Object *) this, viewport, update_mode);
}

void VisualServer::viewport_set_usage(const RID viewport, const int64_t usage) {
	___godot_icall_void_RID_int(___mb.mb_viewport_set_usage, (const Object *) this, viewport, usage);
}

void VisualServer::viewport_set_use_arvr(const RID viewport, const bool use_arvr) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_use_arvr, (const Object *) this, viewport, use_arvr);
}

void VisualServer::viewport_set_vflip(const RID viewport, const bool enabled) {
	___godot_icall_void_RID_bool(___mb.mb_viewport_set_vflip, (const Object *) this, viewport, enabled);
}

}