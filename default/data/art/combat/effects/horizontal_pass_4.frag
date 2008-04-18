// -*- C++ -*-
uniform sampler2D source;

const float texture_width = 8.0;

void main()
{
    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

    const float pixel_size = 1.0 / texture_width;
    color += texture2D(source, gl_TexCoord[0].st + vec2(-3.0 * pixel_size, 0.0)) *  1.0 / 32.0;
    color += texture2D(source, gl_TexCoord[0].st + vec2(-2.0 * pixel_size, 0.0)) *  2.0 / 32.0;
    color += texture2D(source, gl_TexCoord[0].st + vec2(-1.0 * pixel_size, 0.0)) *  4.0 / 32.0;
    color += texture2D(source, gl_TexCoord[0].st + vec2( 0.0 * pixel_size, 0.0)) *  4.0 / 32.0;
    color += texture2D(source, gl_TexCoord[0].st + vec2( 1.0 * pixel_size, 0.0)) *  4.0 / 32.0;
    color += texture2D(source, gl_TexCoord[0].st + vec2( 2.0 * pixel_size, 0.0)) *  2.0 / 32.0;
    color += texture2D(source, gl_TexCoord[0].st + vec2( 3.0 * pixel_size, 0.0)) *  1.0 / 32.0;

    gl_FragColor = color * 0.5;
}
