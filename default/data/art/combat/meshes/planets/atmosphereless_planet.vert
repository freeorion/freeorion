// -*- C++ -*-
uniform vec4 light_pos;

varying float diffuse;
varying vec2 tex_coord;
varying vec3 light_vec;

void main()
{
    vec3 light_dir = normalize(light_pos.xyz);
    diffuse = dot(light_dir, gl_Normal);
    tex_coord = vec2(gl_MultiTexCoord0);

    vec3 tangent = cross(vec3(0.0, 0.0, 1.0), gl_Normal);
    vec3 binormal = cross(gl_Normal, tangent);
    mat3 tangent_space = mat3(tangent, binormal, gl_Normal);
    light_vec = normalize(light_dir * tangent_space);

    gl_Position = ftransform();
}
