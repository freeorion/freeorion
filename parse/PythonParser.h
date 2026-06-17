#ifndef _Python_Parse_h_
#define _Python_Parse_h_

#include "Parse.h"

#include <boost/optional.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/object_fwd.hpp>
#include <filesystem>

class PythonCommon;

struct FO_PARSE_API PythonTypes {
    PythonTypes();
    ~PythonTypes();

    boost::python::object type_int;
    boost::python::object type_float;
    boost::python::object type_bool;
    boost::python::object type_str;
};

class FO_PARSE_API PythonParser {
public:
    PythonParser(PythonCommon& _python);
    ~PythonParser();

    PythonParser(const PythonParser&) = delete;

    PythonParser(PythonParser&&) = delete;

    const PythonParser& operator=(const PythonParser&) = delete;

    PythonParser& operator=(PythonParser&&) = delete;

    /** Parses content file \a path,
      * puts file name from path to \a filename and file content to \a file_contents,
      * return true if parsing was successfull. */
    [[nodiscard]] bool ParseFileCommon(const std::filesystem::path& path,
                         std::string& filename, std::string& file_contents) const;

    using InitFuncPtr = PyObject* (*)();

    [[nodiscard]] boost::python::object LoadModule(InitFuncPtr) const;
    void UnloadModule(boost::python::object module) const;
    void LoadConditionsModule() const;
    void LoadValueRefsModule() const;
    void LoadEffectsModule() const;
    void LoadSourcesModule() const;
    void LoadEnumsModule() const;

private:
    PythonCommon&                  m_python;
    PyThreadState*                 m_parser_thread_state = nullptr;
    PyThreadState*                 m_main_thread_state = nullptr;
    mutable std::unordered_map<InitFuncPtr, boost::python::object> m_initialized_moduiles;
};

#endif

