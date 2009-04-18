#ifdef FREEORION_WIN32
#include <GL/glew.h>
#endif

#include "ShaderProgram.h"

#include <cassert>
#include <iostream>
#include <fstream>


#ifdef NDEBUG
#  define CHECK_ERROR(fn, e)
#else
#  define CHECK_ERROR(fn, e)                                            \
    if ((error = glGetError()) != GL_NO_ERROR)                          \
    {                                                                   \
        std::cerr << fn " () :"                                         \
                  << " GL error on " e ": "                             \
                  << "'" << gluErrorString(error) << "'"                \
                  << std::endl;                                         \
    }
#endif

namespace {
    GLenum error;

    void GetShaderLog(GLuint shader, std::string& log)
    {
        log.clear();
        int logSize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
        CHECK_ERROR("GetShaderLog", "glGetShaderiv(GL_INFO_LOG_LENGTH)");
        if (0 < logSize)
        {
            log.resize(logSize, '\0');
            int chars;
            glGetShaderInfoLog(shader, logSize, &chars, &log[0]);
            CHECK_ERROR("GetShaderLog", "glGetShaderInfoLog()");
        }
    }

    void GetProgramLog(GLuint program, std::string& log)
    {
        log.clear();
        int logSize;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
        CHECK_ERROR("GetProgramLog", "glGetProgramiv(GL_INFO_LOG_LENGTH)");
        if (0 < logSize)
        {
            log.resize(logSize, '\0');
            int chars;
            glGetProgramInfoLog(program, logSize, &chars, &log[0]);
            CHECK_ERROR("GetProgramLog", "glGetProgramInfoLog()");
        }
    }
}

std::string ReadFile(const std::string& filename)
{
    std::string retval;
    std::ifstream ifs(filename.c_str());
    int c;
    while ((c = ifs.get()) != std::ifstream::traits_type::eof()) {
        retval += c;
    }
    return retval;
}

ShaderProgram::ShaderProgram(const std::string& vertex_shader, const std::string& fragment_shader) :
    m_program_id(0),
    m_vertex_shader_id(0),
    m_fragment_shader_id(0),
    m_link_succeeded(false),
    m_program_log(),
    m_program_validity_log(),
    m_vertex_shader_log(),
    m_fragment_shader_log()
{
    assert(!vertex_shader.empty() && !fragment_shader.empty());

    glGetError();

    m_vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glCreateShader(GL_VERTEX_SHADER)");
    m_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glCreateShader(GL_FRAGMENT_SHADER)");

    const char* strings[1] = { 0 };
    strings[0] = vertex_shader.c_str();
    glShaderSource(m_vertex_shader_id, 1, strings, 0);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glShaderSource(vertex_shader)");
    strings[0] = fragment_shader.c_str();
    glShaderSource(m_fragment_shader_id, 1, strings, 0);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glShaderSource(fragment_shader)");

    glCompileShader(m_vertex_shader_id);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glCompileShader(m_vertex_shader_id)");

    glCompileShader(m_fragment_shader_id);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glCompileShader(m_fragment_shader_id)");

    GetShaderLog(m_vertex_shader_id, m_vertex_shader_log);
    GetShaderLog(m_fragment_shader_id, m_fragment_shader_log);

    m_program_id = glCreateProgram();
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glCreateProgram()");

    glAttachShader(m_program_id, m_vertex_shader_id);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glAttachShader(m_vertex_shader_id)");

    glAttachShader(m_program_id, m_fragment_shader_id);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glAttachShader(m_fragment_shader_id)");

    int status;
    glLinkProgram(m_program_id);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glLinkProgram()");
    glGetProgramiv(m_program_id, GL_LINK_STATUS, &status);
    CHECK_ERROR("ShaderProgram::ShaderProgram", "glGetProgramiv(GL_LINK_STATUS)");
    m_link_succeeded = status;

    GetProgramLog(m_program_id, m_program_log);
}

ShaderProgram::~ShaderProgram()
{
    glGetError();
    glDeleteShader(m_vertex_shader_id);
    CHECK_ERROR("ShaderProgram::~ShaderProgram", "glDeleteShader(m_vertex_shader_id)");
    glDeleteShader(m_fragment_shader_id);
    CHECK_ERROR("ShaderProgram::~ShaderProgram", "glDeleteShader(m_fragment_shader_id)");
    glDeleteProgram(m_program_id);
    CHECK_ERROR("ShaderProgram::~ShaderProgram", "glDeleteProgram()");
}

GLuint ShaderProgram::ProgramID() const
{ return m_program_id; }

bool ShaderProgram::LinkSucceeded() const
{ return m_link_succeeded; }

const std::string& ShaderProgram::ProgramInfoLog() const
{ return m_program_log; }

const std::string& ShaderProgram::ProgramValidityInfoLog() const
{ return m_program_validity_log; }

const std::string& ShaderProgram::VertexShaderInfoLog() const
{ return m_vertex_shader_log; }

const std::string& ShaderProgram::FragmentShaderInfoLog() const
{ return m_fragment_shader_log; }

void ShaderProgram::Bind(const std::string& name, float f)
{
    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::Bind", "glGetUniformLocation()");
    assert(location != -1 &&
           "Bind() : The named uniform variable does not exist.");
    glUniform1f(location, f);
    CHECK_ERROR("ShaderProgram::Bind", "glUniform1f()");
}

void ShaderProgram::Bind(const std::string& name, float f0, float f1)
{
    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::Bind", "glGetUniformLocation()");
    assert(location != -1 &&
           "Bind() : The named uniform variable does not exist.");
    glUniform2f(location, f0, f1);
    CHECK_ERROR("ShaderProgram::Bind", "glUniform2f()");
}

void ShaderProgram::Bind(const std::string& name, float f0, float f1, float f2)
{
    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::Bind", "glGetUniformLocation()");
    assert(location != -1 &&
           "Bind() : The named uniform variable does not exist.");
    glUniform3f(location, f0, f1, f2);
    CHECK_ERROR("ShaderProgram::Bind", "glUniform3f()");
}

void ShaderProgram::Bind(const std::string& name, float f0, float f1, float f2, float f3)
{
    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::Bind", "glGetUniformLocation()");
    assert(location != -1 &&
           "Bind() : The named uniform variable does not exist.");
    glUniform4f(location, f0, f1, f2, f3);
    CHECK_ERROR("ShaderProgram::Bind", "glUniform4f()");
}

void ShaderProgram::Bind(const std::string& name, std::size_t element_size, const std::vector<float> &floats)
{
    assert(1 <= element_size && element_size <= 4);
    assert((floats.size() % element_size) == 0);

    typedef void (* UniformFloatArrayFn)(GLint, GLsizei, const GLfloat*);
    UniformFloatArrayFn functions[] =
        { UniformFloatArrayFn(glUniform1fv),
          UniformFloatArrayFn(glUniform2fv),
          UniformFloatArrayFn(glUniform3fv),
          UniformFloatArrayFn(glUniform4fv)
        };

    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::Bind", "glGetUniformLocation()");
    assert(location != -1 &&
           "Bind() : The named uniform variable does not exist.");
    functions[element_size - 1](location,
                                floats.size() / element_size,
                                &floats[0]);
    CHECK_ERROR("ShaderProgram::Bind", "glUniformNfv()");
}

void ShaderProgram::Bind(const std::string& name, GLuint texture_id)
{
    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::Bind", "glGetUniformLocation()");
    assert(location != -1 &&
           "Bind() : The named uniform sampler does not exist.");
    glUniform1i(location, texture_id);
    CHECK_ERROR("ShaderProgram::Bind", "glUniform1i()");
}

void ShaderProgram::BindInt(const std::string& name, int i)
{
    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::BindInt", "glGetUniformLocation()");
    assert(location != -1 &&
           "BindInt() : The named uniform variable does not exist.");
    glUniform1i(location, i);
    CHECK_ERROR("ShaderProgram::BindInt", "glUniform1i()");
}

void ShaderProgram::BindInts(const std::string& name, int i0, int i1)
{
    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::BindInts", "glGetUniformLocation()");
    assert(location != -1 &&
           "BindInts() : The named uniform variable does not exist.");
    glUniform2i(location, i0, i1);
    CHECK_ERROR("ShaderProgram::BindInts", "glUniform2i()");
}

void ShaderProgram::BindInts(const std::string& name, int i0, int i1, int i2)
{
    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::BindInts", "glGetUniformLocation()");
    assert(location != -1 &&
           "BindInts() : The named uniform variable does not exist.");
    glUniform3i(location, i0, i1, i2);
    CHECK_ERROR("ShaderProgram::BindInts", "glUniform3i()");
}

void ShaderProgram::BindInts(const std::string& name, int i0, int i1, int i2, int i3)
{
    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::BindInts", "glGetUniformLocation()");
    assert(location != -1 &&
           "BindInts() : The named uniform variable does not exist.");
    glUniform4i(location, i0, i1, i2, i3);
    CHECK_ERROR("ShaderProgram::BindInts", "glUniform4i()");
}

void ShaderProgram::BindInts(const std::string& name, std::size_t element_size, const std::vector<int> &ints)
{
    assert(1 <= element_size && element_size <= 4);
    assert((ints.size() % element_size) == 0);

    typedef void (* UniformIntegerArrayFn)(GLint, GLsizei, const GLint*);
    UniformIntegerArrayFn functions[] =
        { UniformIntegerArrayFn(glUniform1iv),
          UniformIntegerArrayFn(glUniform2iv),
          UniformIntegerArrayFn(glUniform3iv),
          UniformIntegerArrayFn(glUniform4iv)
        };

    glGetError();
    GLint location = glGetUniformLocation(m_program_id, name.c_str());
    CHECK_ERROR("ShaderProgram::BindInts", "glGetUniformLocation()");
    assert(location != -1 &&
           "BindInts() : The named uniform variable does not exist.");
    functions[element_size - 1](location,
                                ints.size() / element_size,
                                &ints[0]);
    CHECK_ERROR("ShaderProgram::BindInts", "glUniformNiv()");
}

bool ShaderProgram::AllValuesBound()
{
    bool retval = false;
    glGetError();
    glValidateProgram(m_program_id);
    CHECK_ERROR("ShaderProgram::AllValuesBound", "glValidateProgram()");
    int status;
    glGetProgramiv(m_program_id, GL_VALIDATE_STATUS, &status);
    CHECK_ERROR("ShaderProgram::AllValuesBound", "glGetProgramiv(GL_VALIDATE_STATUS)");
    retval = status;
    GetProgramLog(m_program_id, m_program_validity_log);
    return retval;
}

void ShaderProgram::Use()
{
    glGetError();
    glUseProgram(m_program_id);
    CHECK_ERROR("ShaderProgram::Use", "glUseProgram()");
}

