
// GLSL fragment shader that draws horizontal scanlines
uniform float   scanline_spacing;                       // distance between repetitions of pattern

const vec4      half_gray = vec4(0.0, 0.0, 0.0, 0.5);   // darkened pixels
const vec4      transparent = vec4(1.0, 1.0, 1.0, 0.0); // undarkened pixels

void main() {
    gl_FragColor = transparent;
    if (mod(gl_FragCoord.y, scanline_spacing) >= (scanline_spacing / 2.0))
        gl_FragColor = half_gray;
}