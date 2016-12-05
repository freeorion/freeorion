
// GLSL fragment shader that draws horizontal scanlines
uniform float   scanline_spacing;                       // distance between repetitions of pattern
uniform vec4    line_color;                             // color of darkened lines

const vec4      transparent = vec4(1.0, 1.0, 1.0, 0.0); // undarkened pixels

void main() {
    gl_FragColor = transparent;
    if (mod(gl_FragCoord.y, scanline_spacing) >= (scanline_spacing / 2.0))
        gl_FragColor = line_color;
}