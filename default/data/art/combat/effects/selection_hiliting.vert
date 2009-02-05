// -*- C++ -*-
uniform float hiliting_size;
uniform float time;

void main(void)
{
    vec3 pos =
        gl_Vertex.xyz +
        hiliting_size * (1.0 + (sin(time * 5.0) + 1.0) / 5.0) * normalize(gl_Normal);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);
}
