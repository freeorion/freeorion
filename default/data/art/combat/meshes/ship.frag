// -*- C++ -*-
uniform sampler2D color_texture, glow_texture, normal_texture, specular_gloss_texture;
uniform vec3 star_light_color, skybox_light_color;
uniform float alpha;

varying vec3 half_angle;
varying vec3 light_dir;

void main()
{
    vec2 sg = texture2D(specular_gloss_texture, gl_TexCoord[0].st).rg;
    const float MIN_SPECULAR_EXPONENT = 16.0;
    const float MAX_SPECULAR_EXPONENT = 32.0;
    float specular_exponent =
        MIN_SPECULAR_EXPONENT + sg.r * (MAX_SPECULAR_EXPONENT - MIN_SPECULAR_EXPONENT);
    float gloss = sg.g;

    vec3 normal = texture2D(normal_texture, gl_TexCoord[0].st).xyz * 2.0 - 1.0;

    float diffuse = max(dot(normal, light_dir), 0.0);
    float specular = pow(max(dot(normal, half_angle), 0.0), specular_exponent);

    // This acts as "if (diffuse == 0.0) specular = 0.0;", without the branch.
    specular *= sign(max(diffuse, 0.0));

    vec3 diffuse_color = texture2D(color_texture, gl_TexCoord[0].st).rgb;
    vec3 glow_color = texture2D(glow_texture, gl_TexCoord[0].st).rgb;
    vec3 color = max(glow_color, diffuse_color * diffuse + vec3(specular) * gloss);

    gl_FragColor = vec4(color, alpha);
}
