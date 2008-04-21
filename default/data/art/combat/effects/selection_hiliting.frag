// -*- C++ -*-
uniform vec4 hiliting_color;
uniform float time;

void main()
{
    gl_FragColor = hiliting_color;
    gl_FragColor.w *= (sin(time * 5.0) / 2.0 + 1.0) / 2.0;
}
