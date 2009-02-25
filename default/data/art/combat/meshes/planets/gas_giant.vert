// -*- C++ -*-
uniform vec4 light_pos;
uniform vec3 camera_pos;

varying float diffuse;
varying vec2 tex_coord;
varying float position_dot_product;
varying float surface_dot_product;
varying float below_surface_dot_product;

void main()
{
    const float sphere_radius = 1.0;
    const float subsurface_radius = 0.985;

    diffuse = dot(normalize(light_pos.xyz), gl_Normal);
    tex_coord = vec2(gl_MultiTexCoord0);

    vec3 center_vec = normalize(vec3(0.0) - camera_pos.xyz);
    vec3 position_vec = normalize(gl_Vertex.xyz - camera_pos.xyz);

    float view_dist = length(vec3(0.0) - camera_pos.xyz);
    float dot_at_sphere = cos(asin(sphere_radius / view_dist));
    float dot_at_inner_sphere = cos(asin(sphere_radius * subsurface_radius / view_dist));
    float dot_at_position = dot(center_vec, position_vec);

    position_dot_product = dot_at_position;
    surface_dot_product = dot_at_sphere;
    below_surface_dot_product = dot_at_inner_sphere;

    gl_Position = ftransform();
}
