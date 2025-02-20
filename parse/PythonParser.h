#ifndef _Python_Parse_h_
#define _Python_Parse_h_

#include "Parse.h"

#include <boost/optional.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/object_fwd.hpp>
#include <boost/filesystem/path.hpp>

class PythonCommon;

struct module_spec;

class FO_PARSE_API PythonParser {
public:
    PythonParser(PythonCommon& _python, const boost::filesystem::path& scripting_dir);
    ~PythonParser();

    PythonParser(const PythonParser&) = delete;

    PythonParser(PythonParser&&) = delete;

    const PythonParser& operator=(const PythonParser&) = delete;

    PythonParser& operator=(PythonParser&&) = delete;

    /** Parses content file \a path using predefined \a globals,
      * puts file name from path to \a filename and file content to \a file_contents,
      * return true if parsing was successfull. */
    [[nodiscard]] bool ParseFileCommon(const boost::filesystem::path& path,
                         const boost::python::dict& globals,
                         std::string& filename, std::string& file_contents) const;

    boost::python::object type_int;
    boost::python::object type_float;
    boost::python::object type_bool;
    boost::python::object type_str;
private:
    boost::python::object find_spec(const std::string& fullname, const boost::python::object& path, const boost::python::object& target) const;
    boost::python::object create_module(const module_spec& spec);
    boost::python::object exec_module(boost::python::object& module);

    PythonCommon&                  m_python;
    const boost::filesystem::path& m_scripting_dir;
    boost::optional<boost::python::list>            m_meta_path;
    int                            m_meta_path_len;
    PyThreadState*                 m_parser_thread_state = nullptr;
    PyThreadState*                 m_main_thread_state = nullptr;
};

#endif

