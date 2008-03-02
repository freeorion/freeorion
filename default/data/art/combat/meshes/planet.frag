// -*- C++ -*-
uniform sampler2D day_texture, night_texture, cloud_gloss_texture;

varying float diffuse;
varying vec3 specular;
varying vec2 tex_coord;

const float TERMINATOR = 0.3;
const float INV_TERMINATOR = 1.0 / (2.0 * TERMINATOR);

void main()
{
    vec2 cg = texture2D(cloud_gloss_texture, tex_coord).rg;
    float clouds = cg.r;
    float gloss = cg.g;

    vec3 daytime = texture2D(day_texture, tex_coord).rgb * diffuse + specular * gloss;
    daytime = mix(daytime, vec3(abs(diffuse)), clouds);

    vec3 color = daytime;
    if (diffuse < -TERMINATOR)
        color = texture2D(night_texture, tex_coord).rgb * (1.0 - clouds);
    if (abs(diffuse) < TERMINATOR) {
        vec3 nighttime = texture2D(night_texture, tex_coord).rgb * (1.0 - clouds);
        color = mix(nighttime, daytime, (diffuse + TERMINATOR) * INV_TERMINATOR);
    }

    gl_FragColor = vec4(color, 1.0);
}
