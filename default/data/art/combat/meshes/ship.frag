// -*- C++ -*-
uniform sampler2D color_texture, glow_texture, normal_texture, specular_gloss_texture;
uniform vec3 star_light_color, skybox_light_color;

varying float front_diffuse;
varying float back_diffuse;
varying float specular_base;
varying vec2 tex_coord;
varying vec3 light_vec;

void main()
{
    vec2 sg = texture2D(specular_gloss_texture, tex_coord).rg;
    const float MAX_SPECULAR_EXPONENT = 25.0;
    float specular_exponent = sg.r * MAX_SPECULAR_EXPONENT;
    float gloss = 1.0;//sg.g; // TODO: change this when we get a gloss map

    vec3 specular_color = vec3(pow(specular_base, /*specular_exponent*/10.0)) * gloss;
    vec3 diffuse_color =
        texture2D(color_texture, tex_coord).rgb *
        max(front_diffuse * star_light_color, back_diffuse * skybox_light_color);
    vec3 non_glow_color = diffuse_color + specular_color;

    // renormalize to [-1, 1]
    vec3 normal = texture2D(normal_texture, tex_coord).xyz * 2.0 - 1.0;
    float normal_factor = abs(dot(normal, light_vec));
    non_glow_color *= normal_factor;

    vec3 glow_color = texture2D(glow_texture, tex_coord).rgb;
    vec3 color = max(glow_color, non_glow_color);

    gl_FragColor = vec4(color, 1.0);
}
