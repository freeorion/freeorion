// -*- C++ -*-
uniform sampler2D day_texture, night_texture;

varying float diffuse;
varying vec2 tex_coord;

const float TERMINATOR = 0.3;
const float INV_TERMINATOR = 1.0 / (2.0 * TERMINATOR);

void main()
{
    vec3 daytime = texture2D(day_texture, tex_coord).rgb * diffuse;
    vec3 color = daytime;
    if (diffuse < -TERMINATOR)
        color = texture2D(night_texture, tex_coord).rgb;
    if (abs(diffuse) < TERMINATOR) {
        vec3 nighttime = texture2D(night_texture, tex_coord).rgb;
        color = mix(nighttime, daytime, (diffuse + TERMINATOR) * INV_TERMINATOR);
    }

    gl_FragColor = vec4(color, 1.0);
}
