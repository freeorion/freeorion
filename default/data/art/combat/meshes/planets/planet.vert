// -*- C++ -*-
uniform vec3 light_dir;

varying float diffuse;
varying vec3 specular;
varying vec2 tex_coord;

void main()
{
    vec4 ec_position = gl_ModelViewMatrix * gl_Vertex;
    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    vec3 light_vec = normalize(gl_NormalMatrix * light_dir);
    vec3 reflect_vec = reflect(-light_vec, normal);
    vec3 view_vec = normalize(vec3(-ec_position));

    float spec_intensity = pow(max(dot(reflect_vec, view_vec), 0.0), 10.0);
    specular = vec3(spec_intensity);
    diffuse = dot(light_vec, normal);
    tex_coord = vec2(gl_MultiTexCoord0);

    gl_Position = ftransform();
}
