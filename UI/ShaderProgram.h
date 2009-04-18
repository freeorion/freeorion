// -*- C++ -*-
#ifndef _ShaderProgram_h_
#define _ShaderProgram_h_

#ifdef _MSC_VER
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

// include OpenGL headers
#if defined(__APPLE__) && defined(__MACH__)
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
#endif

#include <string>
#include <vector>


std::string ReadFile(const std::string& filename);

class ShaderProgram
{
public:
    ShaderProgram(const std::string& vertex_shader, const std::string& fragment_shader);
    ~ShaderProgram();

    GLuint ProgramID() const;
    bool LinkSucceeded() const;
    const std::string& ProgramInfoLog() const;
    const std::string& ProgramValidityInfoLog() const;
    const std::string& VertexShaderInfoLog() const;
    const std::string& FragmentShaderInfoLog() const;

    // These bind the value of the uniform float(s) referred called \a name in
    // the program.  Multi-float overloads are for when \a name is a vec.
    void Bind(const std::string& name, float f);
    void Bind(const std::string& name, float f0, float f1);
    void Bind(const std::string& name, float f0, float f1, float f2);
    void Bind(const std::string& name, float f0, float f1, float f2, float f3);

    // Binds the given array of values to the int array \a name in the
    // program.  \a element_size indicates the number of
    // ints in each element of the array, and must be 1, 2, 3, or 4
    // (corresponding to GLSL types float, vec2, vec3, and vec4).
    void Bind(const std::string& name, std::size_t element_size, const std::vector<float> &floats);

    void Bind(const std::string& name, GLuint texture_id);

    // These bind the value of the uniform int(s) referred called \a name in
    // the program.  Multi-int overloads are for when \a name is an ivec.
    void BindInt(const std::string& name, int i);
    void BindInts(const std::string& name, int i0, int i1);
    void BindInts(const std::string& name, int i0, int i1, int i2);
    void BindInts(const std::string& name, int i0, int i1, int i2, int i3);

    // Binds the given array of values to the int array \a name in the
    // program.  \a element_size indicates the number of
    // ints in each element of the array, and must be 1, 2, 3, or 4
    // (corresponding to GLSL types int, ivec2, ivec3, and ivec4).
    void BindInts(const std::string& name, std::size_t element_size, const std::vector<int> &ints);

    bool AllValuesBound();

    void Use();

private:
    GLuint      m_program_id;
    GLuint      m_vertex_shader_id;
    GLuint      m_fragment_shader_id;
    bool        m_link_succeeded;
    std::string m_program_log;
    std::string m_program_validity_log;
    std::string m_vertex_shader_log;
    std::string m_fragment_shader_log;
};

#endif
