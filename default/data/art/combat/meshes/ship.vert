// -*- C++ -*-
uniform vec3 light_dir;

varying float front_diffuse; // diffuse color from key light (light_dir, i.e. the star)
varying float back_diffuse; // diffuse color from fill light (-light_dir, i.e. the skybox)
varying float specular_base;
varying vec2 tex_coord;
varying vec3 light_vec;

void main()
{
    vec4 ec_position = gl_ModelViewMatrix * gl_Vertex;
    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    light_vec = normalize(gl_NormalMatrix * light_dir);
    vec3 reflect_vec = reflect(-light_vec, normal);
    vec3 view_vec = normalize(vec3(-ec_position));

    specular_base = max(dot(reflect_vec, view_vec), 0.0);
    front_diffuse = dot(light_vec, normal);
    back_diffuse = dot(-light_vec, normal);
    tex_coord = vec2(gl_MultiTexCoord0);

    vec3 tangent = cross(vec3(0.0, 0.0, 1.0), gl_Normal);
    vec3 binormal = cross(gl_Normal, tangent);
    mat3 tangent_space = mat3(tangent, binormal, gl_Normal);
    light_vec = normalize(light_dir * tangent_space);

    gl_Position = ftransform();
}
