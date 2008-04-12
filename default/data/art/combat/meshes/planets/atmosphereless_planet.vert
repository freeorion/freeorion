// -*- C++ -*-
uniform vec3 light_dir;

varying float diffuse;
varying vec2 tex_coord;

void main()
{
    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    vec3 light_vec = normalize(gl_NormalMatrix * light_dir);

    diffuse = dot(light_vec, normal);
    tex_coord = vec2(gl_MultiTexCoord0);

    gl_Position = ftransform();
}
