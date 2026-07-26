#include "PythonCommon.h"

#include <boost/algorithm/string.hpp>
#include <boost/python/module.hpp>

#include "../util/Directories.h"
#include "../util/Logger.h"

#ifdef FREEORION_WIN32
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>
#endif

namespace py = boost::python;

namespace {
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
    wchar_t* GetFilePath(const std::filesystem::path& path)
    {
# if defined(FREEORION_WIN32)
        const std::wstring_view native_path = path.native();
        wchar_t* py_path = (wchar_t*)PyMem_RawCalloc(native_path.size() + 1, sizeof(wchar_t));
        native_path.copy(py_path, native_path.size());
        py_path[native_path.size()] = L'\0';
        return py_path;
# else
        return Py_DecodeLocale(path.string().c_str(), nullptr);
# endif
    }
#endif

    auto GetLoggableString(const wchar_t* const original)
    {
#if defined(FREEORION_WIN32)
        const std::wstring_view view(original);
        const size_t original_size = view.size(); // passing -1 would compute size for null terminated string, but would include the null
        int utf8_size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, original, original_size, nullptr, 0, nullptr, nullptr);
        std::string utf8_string(utf8_size, '\0');
        if (utf8_size > 0)
            WideCharToMultiByte(CP_UTF8, 0, original, original_size, utf8_string.data(), utf8_size, nullptr, nullptr);
        return utf8_string;
#else
        return original;
#endif
    }

    template<typename T = std::filesystem::path::value_type>
    py::object path_to_pyobject(const std::basic_string<T>& filename) {
        PyObject* raw_py_str = nullptr;
        if constexpr (std::is_same_v<T, wchar_t>)
            raw_py_str = PyUnicode_FromWideChar(filename.c_str(), filename.size());
        else
            raw_py_str = PyUnicode_FromStringAndSize(filename.c_str(), filename.size());
        if (!raw_py_str)
            return py::object();
        return py::object(py::handle<>(raw_py_str));
    }

    template<typename T = std::filesystem::path::value_type>
    std::basic_string<T> pyobject_to_path(const py::object& o_filename) {
        PyObject* raw_obj = o_filename.ptr();
        if (!raw_obj || !PyUnicode_Check(raw_obj))
            return {};
        if constexpr (std::is_same_v<T, wchar_t>) {
            Py_ssize_t size = 0;
            wchar_t* buffer = PyUnicode_AsWideCharString(raw_obj, &size);
            if (!buffer)
                return {};
            std::basic_string<T> result(buffer, size);
            PyMem_Free(buffer);
            return result;
        } else {
            Py_ssize_t size = 0;
            const char* buffer = PyUnicode_AsUTF8AndSize(raw_obj, &size);
            if (!buffer)
                return {};
            return std::basic_string<T>(buffer, size);
        }
    }

    auto GetPythonExecutable() // should be the containing C++ binary / .exe file
    { return py::extract<std::string>(py::import("sys").attr("executable"))(); }

    struct import_error : std::runtime_error {
        import_error(const std::string& what) : std::runtime_error(what) {}
    };

    void translate(import_error const& e) {
        PyErr_SetString(PyExc_ImportError, e.what());
    }

    constexpr bool STATIC_FALSE = false;
    constexpr bool STATIC_TRUE = true;
}

struct module_spec {
    py::object submodule_search_locations;
    py::object origin;
    py::list uninitialized_submodules;
    std::string fullname;
    std::string parent;
    const PythonCommon& python;
};

BOOST_PYTHON_MODULE(freeorion_loader) {
    py::register_exception_translator<import_error>(&translate);

    py::class_<PythonCommon, py::bases<>, PythonCommon, boost::noncopyable>("PythonCommon", py::no_init)
        .def("find_spec", &PythonCommon::find_spec)
        .def("create_module", &PythonCommon::create_module)
        .def("exec_module", &PythonCommon::exec_module);

    py::class_<module_spec>("PythonCommonModuleSpec", py::no_init)
        .def_readonly("name", &module_spec::fullname)
        .def_readonly("_uninitialized_submodules", &module_spec::uninitialized_submodules)
        .add_property("loader", py::make_function(+[](const module_spec& self) -> const PythonCommon& { return self.python; }, py::return_value_policy<py::reference_existing_object>()))
        .def_readonly("submodule_search_locations", &module_spec::submodule_search_locations)
        .def_readonly("origin", &module_spec::origin)
        .def_readonly("has_location", STATIC_TRUE)
        .def_readonly("cached", STATIC_FALSE)
        .def_readonly("parent", &module_spec::parent);
}

PythonCommon::~PythonCommon()
{ Finalize(); }

bool PythonCommon::IsPythonRunning() const
{ return Py_IsInitialized(); }

bool PythonCommon::Initialize() {
    DebugLogger() << "Initializing FreeOrion Python interface";

    try {
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
        // There have been recurring issues on Windows and OSX to get FO to use the
        // Python framework shipped with the app (instead of falling back on the ones
        // provided by the system). These API calls have been added in an attempt to
        // solve the problems. Not sure if they are really required, but better safe
        // than sorry... ;)

        m_home_dir = GetFilePath(GetPythonHome());
        Py_SetPythonHome(m_home_dir);
        DebugLogger() << "Python home set to " << GetLoggableString(Py_GetPythonHome());

        m_program_name = GetFilePath(GetPythonHome() / "Python");
        Py_SetProgramName(m_program_name);
        DebugLogger() << "Python program name set to " << GetLoggableString(Py_GetProgramName());
#endif

#if defined(FREEORION_ANDROID)
        Py_NoSiteFlag = 1;
#endif
        // allow the "freeorion_loader" C++ module to be imported within Python code
        if (PyImport_AppendInittab("freeorion_loader", PyInit_freeorion_loader) == -1) {
            ErrorLogger() << "Unable to initialize freeorion_loader import";
            return false;
        }

        if (!InitCommonImports()) {
            ErrorLogger() << "Unable to initialize imports";
            return false;
        }
        // initializes Python interpreter, allowing Python functions to be called from C++
        Py_Initialize();
        DebugLogger() << "Python initialized";
        DebugLogger() << "Python program: " << GetPythonExecutable();
        DebugLogger() << "Python version: " << Py_GetVersion();
        auto sys = py::import("sys");
        DebugLogger() << "Python prefix: " << py::extract<std::string>(sys.attr("base_prefix"))();
        DebugLogger() << "Python module search path: " << py::extract<std::string>(py::str(sys.attr("path")))();
    }
    catch (...) {
        ErrorLogger() << "Unable to initialize Python interpreter";
        return false;
    }

    return true;
}

bool PythonCommon::InitModuleLoader() {
    if (!m_meta_path) {
        try {
            py::import("freeorion_loader");
            m_meta_path = py::extract<py::list>(py::import("sys").attr("meta_path"))();
            m_meta_path->insert(0, boost::cref(*this));
            m_meta_path_len = static_cast<int>(py::len(*m_meta_path));
        } catch (const py::error_already_set&) {
            HandleErrorAlreadySet();
            ErrorLogger() << "Unable to initialize Python module loader because of Python errors";
            return false;
        } catch (...) {
            ErrorLogger() << "Unable to initialize Python module loader";
            return false;
        }
    }

    return true;
}

bool PythonCommon::InitErrorHandler() {
    try {
        m_system_exit = py::import("builtins").attr("SystemExit");
        m_traceback_format_exception = py::import("traceback").attr("format_exception");
    } catch (const py::error_already_set&) {
        HandleErrorAlreadySet();
        ErrorLogger() << "Unable to initialize FreeOrion Python SystemExit";
        return false;
    } catch (...) {
        ErrorLogger() << "Unable to initialize FreeOrion Python SystemExit";
        return false;
    }

    return true;
}

bool PythonCommon::InitCommonImports()
{ return true; }

void PythonCommon::HandleErrorAlreadySet() {
    if (!Py_IsInitialized()) {
        ErrorLogger() << "Python interpreter not initialized and exception handler called.";
        return;
    }

    // Matches system exit
    if (PyErr_ExceptionMatches(m_system_exit.ptr()))
    {
        Finalize();
        ErrorLogger() << "Python interpreter exited with SystemExit(), sys.exit(), exit, quit or some other alias.";
        return;
    }

    PyObject *extype, *value, *traceback;
    PyErr_Fetch(std::addressof(extype), std::addressof(value), std::addressof(traceback));
    PyErr_NormalizeException(std::addressof(extype), std::addressof(value), std::addressof(traceback));
    if (!extype) {
        ErrorLogger() << "Missing python exception type";
        return;
    }

    py::object o_extype(py::handle<>(py::borrowed(extype)));
    py::object o_value(py::handle<>(py::borrowed(value)));
    py::object o_traceback = (traceback != nullptr) ?
        py::object(py::handle<>(py::borrowed(traceback))) : py::object();

    py::object lines = m_traceback_format_exception(o_extype, o_value, o_traceback);
    for (int i = 0; i < len(lines); ++i) {
        std::string line = py::extract<std::string>(lines[i])();
        boost::algorithm::trim_right(line);
        ErrorLogger() << line;
    }
    return;
}

void PythonCommon::Finalize() {
    FinalizeModuleLoader();
    if (Py_IsInitialized()) {
        // cleanup python objects before interpterer shutdown
        m_system_exit = py::object();
        m_traceback_format_exception = py::object();
        try {
            // According to boost.python 1.69 docs python Py_Finalize must not be called
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32)
            if (m_home_dir != nullptr) {
                PyMem_RawFree(m_home_dir);
                m_home_dir = nullptr;
            }
            if (m_program_name != nullptr) {
                PyMem_RawFree(m_program_name);
                m_program_name = nullptr;
            }
#endif
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception when cleaning up FreeOrion Python interface: " << e.what();
            return;
        } catch (...) {
            ErrorLogger() << "Caught unknown exception when cleaning up FreeOrion Python interface";
        }
    }
}

void PythonCommon::FinalizeModuleLoader() {
    if (Py_IsInitialized()) {
        if (m_meta_path) {
            try {
                m_meta_path->pop(0);
                m_meta_path = boost::none;
            } catch (const py::error_already_set&) {
                ErrorLogger() << "Python parser destructor throw exception";
                HandleErrorAlreadySet();
            }
        }
    }
}

void PythonCommon::CompileEval(const char* code, const std::filesystem::path& filename, const py::object& globals) {
    py::object o_filename_str = path_to_pyobject(filename.native());
    if (o_filename_str.is_none()) {
        ErrorLogger() << "Failed to convert path to str: " << PathToString(filename);
        py::throw_error_already_set();
    }
    PyObject* compiled_code = Py_CompileStringObject(code, o_filename_str.ptr(), Py_file_input, nullptr, 2);
    if (!compiled_code) {
        ErrorLogger() << "Failed to compile: " << PathToString(filename);
        py::throw_error_already_set();
    }
    py::object o_code{py::handle<>(compiled_code)};
    PyObject* result = PyEval_EvalCode(o_code.ptr(), globals.ptr(), globals.ptr());
    if (!result) {
        ErrorLogger() << "Failed to eval: " << PathToString(filename);
        py::throw_error_already_set();
    }
    py::object o_result{py::handle<>(result)};
}

void PythonCommon::SetModulesDirs(const std::vector<std::filesystem::path>& modules_dirs) {
    m_modules_dirs = modules_dirs;
}

void PythonCommon::SetModulesDirs(std::vector<std::filesystem::path>&& modules_dirs) {
    m_modules_dirs = std::move(modules_dirs);
}

py::object PythonCommon::find_spec(const std::string& fullname, const py::object& path, const py::object& target) const {
    std::vector<std::filesystem::path> namespace_dirs;
    std::string parent;
    std::string current;
    for (auto it = boost::algorithm::make_split_iterator(fullname, boost::algorithm::token_finder(boost::algorithm::is_any_of(".")));
        it != boost::algorithm::split_iterator<std::string::const_iterator>(); ++it)
    {
        if (!current.empty()) {
            if (parent.empty())
                parent = std::move(current);
            else
                parent = parent + "." + current;
        }
        current = boost::copy_range<std::string>(*it);
    }
    for (auto module_path : m_modules_dirs) {
        for (auto it = boost::algorithm::make_split_iterator(fullname, boost::algorithm::token_finder(boost::algorithm::is_any_of(".")));
            it != boost::algorithm::split_iterator<std::string::const_iterator>(); ++it)
        {
            module_path = module_path / boost::copy_range<std::string>(*it);
        }

        if (IsExistingDir(module_path)) {
            auto init_py_path = module_path / "__init__.py";
            py::list search_locations;
            search_locations.append(path_to_pyobject(module_path.native()));
            if (IsExistingFile(init_py_path)) {
                return py::object(module_spec{
                    .submodule_search_locations = search_locations,
                    .origin = path_to_pyobject(init_py_path.native()),
                    .fullname = fullname,
                    .parent = parent,
                    .python = *this
                });
            }
            namespace_dirs.push_back(module_path);
        } else {
            module_path.replace_extension("py");
            if (IsExistingFile(module_path)) {
                return py::object(module_spec{
                    .submodule_search_locations = py::object(), // None for standard files
                    .origin = path_to_pyobject(module_path.native()),
                    .fullname = fullname,
                    .parent = parent,
                    .python = *this
                });
            }
        }
    }

    if (!namespace_dirs.empty()) {
        py::list search_locations;
        for (const auto& dir : namespace_dirs) {
            search_locations.append(path_to_pyobject(dir.native()));
        }
        return py::object(module_spec{
            .submodule_search_locations = search_locations,
            .origin = py::object(),
            .fullname = fullname,
            .parent = parent,
            .python = *this
        });
    }

    WarnLogger() << "Couldn't find file for module spec " << fullname;
    return py::object();
}

py::object PythonCommon::create_module(const module_spec& spec)
{ return py::object(); }

py::object PythonCommon::exec_module(py::object& module) {
    std::string fullname = py::extract<std::string>(module.attr("__name__"));

    py::dict globals = py::extract<py::dict>(module.attr("__dict__"));

    if (!globals.contains("__file__"))
        return py::object();
    py::object py_module_path = globals["__file__"];
    if (py_module_path.is_none())
        return py::object();
    auto module_path = std::filesystem::path(pyobject_to_path(py_module_path));

    if (IsExistingFile(module_path)) {
        std::string file_contents;
        bool read_success = ReadFile(module_path, file_contents);
        if (!read_success) {
            ErrorLogger() << "Unable to open data file " << module_path.string();
            throw import_error("Unreadable module " + fullname);
        }

        if (globals.contains("__spec__")) {
            py::object py_spec = globals["__spec__"];
            auto opt_spec = py::extract<const module_spec&>(py_spec);
            if (opt_spec.check()) {
                const module_spec& spec = opt_spec();
                if (spec.parent.empty()) {
                    if (module_path.filename() == "__init__.py") {
                        globals["__package__"] = fullname;
                    } else {
                        size_t last_dot = fullname.find_last_of('.');
                        globals["__package__"] = (last_dot != std::string::npos) ? fullname.substr(0, last_dot) : "";
                    }
                } else {
                    globals["__package__"] = spec.parent;
                }
            } else {
                WarnLogger() << "Wrong spec in module " << module_path.string();
            }
        } else {
            WarnLogger() << "No spec in module " << module_path.string();
        }

        // store globals content in module namespace
        // it is required so functions in the same module will see each other
        // and still import will work
        DebugLogger() << "Executing module file " << module_path.string();
        try {
            CompileEval(file_contents.c_str(), module_path.native(), globals);
        } catch (const boost::python::error_already_set&) {
            HandleErrorAlreadySet();
            ErrorLogger() << "Unable to parse module file " << PathToString(module_path);
            if (!IsPythonRunning()) {
                ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
                if (Initialize()) {
                    ErrorLogger() << "Python interpreter successfully restarted.";
                } else {
                    ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
                }
            }
            throw import_error("Cannot execute module " + fullname);
        }

        return py::object();
    } else {
        throw import_error("Module not existed " + fullname);
    }
}
