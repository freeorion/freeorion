// -*- C++ -*-
uniform sampler2D color_texture, glow_decal_texture, normal_specular_texture;
uniform vec3 star_light_color, skybox_light_color;
uniform vec3 decal_color;
uniform float alpha;

varying vec3 star_half_angle;
varying vec3 skybox_half_angle;
varying vec3 light_dir;

#define BlendOverlayf(base, blend) (base < 0.5 ? (2.0 * base * blend) : (1.0 - 2.0 * (1.0 - base) * (1.0 - blend)))

void main()
{
    vec4 color_texel = texture2D(color_texture, gl_TexCoord[0].st);
    vec4 glow_decal_texel = texture2D(glow_decal_texture, gl_TexCoord[0].st);
    vec4 normal_specular_texel = texture2D(normal_specular_texture, gl_TexCoord[0].st);

    float specular_and_gloss = normal_specular_texel.a;
    const float MIN_SPECULAR_EXPONENT = 16.0;
    const float MAX_SPECULAR_EXPONENT = 32.0;
    float specular_exponent =
        MIN_SPECULAR_EXPONENT + specular_and_gloss * (MAX_SPECULAR_EXPONENT - MIN_SPECULAR_EXPONENT);
    float gloss = specular_and_gloss;

    vec3 normal = normal_specular_texel.xyz * 2.0 - 1.0;

    float star_diffuse = max(dot(normal, light_dir), 0.0);
    float star_specular = pow(max(dot(normal, star_half_angle), 0.0), specular_exponent);

    float skybox_diffuse = max(-dot(normal, light_dir), 0.0);
    float skybox_specular = pow(max(dot(normal, skybox_half_angle), 0.0), specular_exponent);

    // These lines act as "if ([...]_diffuse == 0.0) [...]_specular = 0.0;",
    // without the branch.
    star_specular *= sign(max(star_diffuse, 0.0));
    skybox_specular *= sign(max(skybox_diffuse, 0.0));

    vec3 overlay_color = vec3(BlendOverlayf(glow_decal_texel.a, decal_color.r),
                              BlendOverlayf(glow_decal_texel.a, decal_color.g),
                              BlendOverlayf(glow_decal_texel.a, decal_color.b));
    vec3 hull_color = max(color_texel.rgb, overlay_color);

    const float STAR_LIGHT_BIAS = 1.5;
    const float SKYBOX_LIGHT_BIAS = 1.2;

    vec3 star_light_contribution =
        (hull_color * star_diffuse + vec3(star_specular) * gloss) *
        star_light_color * STAR_LIGHT_BIAS;
    vec3 skybox_light_contribution =
        (hull_color * skybox_diffuse + vec3(skybox_specular) * gloss) *
        skybox_light_color * SKYBOX_LIGHT_BIAS;

    vec3 glow_color = glow_decal_texel.rgb;
    vec3 color = max(glow_color, star_light_contribution + skybox_light_contribution);

    gl_FragColor = vec4(color, color_texel.a * alpha);
}
