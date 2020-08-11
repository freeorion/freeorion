#ifndef _ShaderProgram_h_
#define _ShaderProgram_h_

#include <GG/Base.h>

#include <boost/filesystem/operations.hpp>

#include <memory>
#include <string>
#include <vector>

bool ReadFile(const boost::filesystem::path& path, std::string& file_contents);

class ShaderProgram {
private:
    ShaderProgram();    // default ctor forbidden, makes no sense

public:
    // Use shaderProgramFactory() to construct.
    ShaderProgram(const std::string& vertex_shader, const std::string& fragment_shader);

    // shader factory -- will return nullptr if OpenGL version is too low
    static std::unique_ptr<ShaderProgram> shaderProgramFactory(
        const std::string& vertex_shader, const std::string& fragment_shader);

    ~ShaderProgram();

    GLuint              ProgramID() const;
    bool                LinkSucceeded() const;
    const std::string&  ProgramInfoLog() const;
    const std::string&  ProgramValidityInfoLog() const;
    const std::string&  VertexShaderInfoLog() const;
    const std::string&  FragmentShaderInfoLog() const;

    // These bind the value of the uniform float(s) referred called \a name in
    // the program.  Multi-float overloads are for when \a name is a vec.
    void                Bind(const std::string& name, float f);
    void                Bind(const std::string& name, float f0, float f1);
    void                Bind(const std::string& name, float f0, float f1, float f2);
    void                Bind(const std::string& name, float f0, float f1, float f2, float f3);

    // Binds the given array of values to the int array \a name in the
    // program.  \a element_size indicates the number of
    // ints in each element of the array, and must be 1, 2, 3, or 4
    // (corresponding to GLSL types float, vec2, vec3, and vec4).
    void                Bind(const std::string& name, std::size_t element_size, const std::vector<float> &floats);

    void                Bind(const std::string& name, GLuint texture_id);

    // These bind the value of the uniform int(s) referred called \a name in
    // the program.  Multi-int overloads are for when \a name is an ivec.
    void                BindInt(const std::string& name, int i);
    void                BindInts(const std::string& name, int i0, int i1);
    void                BindInts(const std::string& name, int i0, int i1, int i2);
    void                BindInts(const std::string& name, int i0, int i1, int i2, int i3);

    // Binds the given array of values to the int array \a name in the
    // program.  \a element_size indicates the number of
    // ints in each element of the array, and must be 1, 2, 3, or 4
    // (corresponding to GLSL types int, ivec2, ivec3, and ivec4).
    void                BindInts(const std::string& name, std::size_t element_size, const std::vector<GLint> &ints);

    bool                AllValuesBound();

    void                Use();
    void                stopUse();

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
