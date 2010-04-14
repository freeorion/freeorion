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

#if 0
    light_dir = normalize(light_pos) * tangent_space;
#else
    // The math commented out above is correct.  However, the art folks want
    // to light ships from above the ecliptic, because it looks better that
    // way.  This fake light_dir is derived by rotating light_pos up off the
    // ecliptic by a fixed angle.

    vec3 normalized_light_pos = normalize(light_pos);
    vec3 axis = cross(normalized_light_pos, vec3(0.0, 0.0, 1.0));

    const float THETA = radians(15.0);
    const float COS = cos(THETA);
    const float SIN = sin(THETA);
    const float ONE_MINUS_COS = 1.0 - COS;
    const float XX = axis.x * axis.x;
    const float YY = axis.y * axis.y;
    const float ZZ = axis.z * axis.z;
    const float XYM = axis.x * axis.y * ONE_MINUS_COS;
    const float XZM = axis.x * axis.z * ONE_MINUS_COS;
    const float YZM = axis.y * axis.z * ONE_MINUS_COS;
    const float X_SIN = axis.x * SIN;
    const float Y_SIN = axis.y * SIN;
    const float Z_SIN = axis.z * SIN;
    mat3 rotate_up = mat3(
        XX * ONE_MINUS_COS + COS, XYM + Z_SIN,              XZM - Y_SIN,
        XYM - Z_SIN,              YY * ONE_MINUS_COS + COS, YZM + X_SIN,
        XZM + Y_SIN,              YZM - X_SIN,              ZZ * ONE_MINUS_COS + COS
    );

    light_dir = rotate_up * normalized_light_pos * tangent_space;
#endif

    star_half_angle = normalize(eye_dir + light_dir);
    skybox_half_angle = normalize(eye_dir + -light_dir);

    gl_TexCoord[0].st = vec2(gl_MultiTexCoord0);
    gl_Position = ftransform();
}
