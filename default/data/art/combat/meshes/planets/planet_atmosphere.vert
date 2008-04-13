// -*- C++ -*-
uniform vec3 light_dir;
uniform vec3 camera_pos;

varying float sun_intensity;
varying float position_dot_product;
varying float above_surface_dot_product;
varying float surface_dot_product;
varying float edge_below_surface_dot_product;
varying float below_surface_dot_product;

void main()
{
    const float sphere_radius = 1.0;
    const float atmosphere_radius = 1.05;
    const float edge_subsurface_radius = 0.99;
    const float subsurface_radius = 0.4;

    vec4 vertex = vec4(gl_Vertex.xyz * atmosphere_radius, 1.0);

    vec3 normal = normalize(gl_NormalMatrix * normalize(gl_Normal));
    vec3 light_vec = normalize(gl_NormalMatrix * light_dir);

    sun_intensity = max(dot(normal, light_vec), 0.0);

    vec3 center_vec = normalize(vec3(0.0) - camera_pos.xyz);
    vec3 position_vec = normalize(vertex.xyz - camera_pos.xyz);

    float view_dist = length(vec3(0.0) - camera_pos.xyz);
    above_surface_dot_product = cos(asin(sphere_radius * atmosphere_radius / view_dist));
    surface_dot_product = cos(asin(sphere_radius / view_dist));
    edge_below_surface_dot_product = cos(asin(sphere_radius * edge_subsurface_radius / view_dist));
    below_surface_dot_product = cos(asin(sphere_radius * subsurface_radius / view_dist));
    position_dot_product = dot(center_vec, position_vec);

    gl_Position = gl_ModelViewProjectionMatrix * vertex;
}
