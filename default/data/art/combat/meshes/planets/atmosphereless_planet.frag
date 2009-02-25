// -*- C++ -*-
uniform sampler2D day_texture, night_texture, normal_texture, lights_texture;

varying float diffuse;
varying vec2 tex_coord;
varying vec3 light_vec;

const float TERMINATOR = 0.3;

void main()
{
    vec3 color = texture2D(day_texture, tex_coord).rgb * diffuse;

    // renormalize to [-1, 1]
    vec3 normal = texture2D(normal_texture, tex_coord).xyz * 2.0 - 1.0;
    float normal_factor = max(dot(normal, light_vec), 0.0);
    color *= normal_factor;

    if (diffuse < -TERMINATOR) {
        vec3 nighttime = texture2D(night_texture, tex_coord).rgb;
        vec3 city_lights = texture2D(lights_texture, tex_coord).rgb;
        color = clamp(nighttime + city_lights, 0.0, 1.0);
    }
    if (abs(diffuse) < TERMINATOR) {
        vec3 nighttime = texture2D(night_texture, tex_coord).rgb;
        vec3 city_lights = texture2D(lights_texture, tex_coord).rgb;
        vec3 combined_night_side = clamp(nighttime + city_lights, 0.0, 1.0);
        color = mix(combined_night_side, color,
                    (diffuse + TERMINATOR) / (2.0 * TERMINATOR));
    }

    gl_FragColor = vec4(color, 1.0);
}
