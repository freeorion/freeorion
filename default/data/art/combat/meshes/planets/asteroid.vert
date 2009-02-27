// -*- C++ -*-
uniform vec4 light_pos;

varying float diffuse;
varying float alpha;
varying vec2 tex_coord;
varying vec3 light_vec;

// We don't use some of these, but PagedGeometry expects this interface.
uniform vec4 objSpaceLight;
uniform vec4 lightDiffuse;
uniform vec4 lightAmbient;
uniform mat4 worldViewProj;
uniform vec3 camPos;
uniform float invisibleDist;
uniform float fadeGap;

void main()
{
    vec3 light_dir = normalize(light_pos.xyz);
    diffuse = dot(light_dir, gl_Normal);
    tex_coord = vec2(gl_MultiTexCoord0);

    vec3 tangent = cross(vec3(0.0, 0.0, 1.0), gl_Normal);
    vec3 binormal = cross(gl_Normal, tangent);
    mat3 tangent_space = mat3(tangent, binormal, gl_Normal);
    light_vec = normalize(light_dir * tangent_space);

    float dist = distance(camPos.xz, gl_Vertex.xz);
    // This is disabled because it causes the PagedGeometry impostors to be 0%
    // alpha.
    //alpha = (invisibleDist - dist) / fadeGap;
    alpha = 1.0;

    gl_Position = ftransform();
}
