// -*- C++ -*-
uniform vec3 light_pos;
uniform vec3 camera_pos;

varying vec3 star_half_angle;
varying vec3 skybox_half_angle;
varying vec3 light_dir;

void main()
{
    vec3 tangent = cross(vec3(0.0, 0.0, 1.0), gl_Normal);
    vec3 binormal = cross(gl_Normal, tangent);
    mat3 tangent_space = mat3(tangent, binormal, gl_Normal);
    vec3 eye_dir = normalize(camera_pos) * tangent_space;

    light_dir = normalize(light_pos) * tangent_space;
    star_half_angle = normalize(eye_dir + light_dir);
    skybox_half_angle = normalize(eye_dir + -light_dir);

    gl_TexCoord[0].st = vec2(gl_MultiTexCoord0);
    gl_Position = ftransform();
}
