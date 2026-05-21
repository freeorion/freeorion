#ifndef __FreeOrion__Util__PythonCommon__
#define __FreeOrion__Util__PythonCommon__

#include <filesystem>

#include "../util/boost_fix.h"
#include <boost/python.hpp>
#include <boost/optional.hpp>

#include "Export.h"

struct module_spec;

class FO_COMMON_API PythonCommon {
public:
    PythonCommon() = default;
    virtual ~PythonCommon();

    /** IsPythonRunning returns true is the python interpreter is
        initialized.  It is typically called after
        HandleErrorAlreadySet() to determine if the error caused the
        interpreter to shutdown. */
    bool IsPythonRunning() const;

    /** Handles boost::python::error_already_set.
        If the error is SystemExit the python interpreter is finalized
        and no longer available.

        Call PyErr_Print() if the exception is an error.

        HandleErrorAlreadySet is idempotent, calling it multiple times
        won't crash or hang the process. */
    void HandleErrorAlreadySet();

    bool Initialize();        // initializes and runs the Python interpreter, prepares the Python environment

    bool InitErrorHandler();  // initializes error handler

    bool InitModuleLoader();  // initializes module loader

    virtual bool InitCommonImports(); // initializes Python imports

    void Finalize();          // stops Python interpreter and releases its resources

    void FinalizeModuleLoader();  // removes module loader

    // Compiles and evaluates \a code with defined \a filename. Populates \a globals
    static void CompileEval(const char* code, const std::filesystem::path& filename, const boost::python::object& globals);

    void SetModulesDir(const std::filesystem::path& modules_dir);

    void SetPopulateGlobalsFunc(std::function<void(boost::python::dict&)> populate_globals_func);

    //! @name Modules finder and loader
    //! Methods exposed to Python as a meta path finder and a loader
    //! @{
    boost::python::object find_spec(const std::string& fullname, const boost::python::object& path, const boost::python::object& target) const;
    boost::python::object create_module(const module_spec& spec);
    boost::python::object exec_module(boost::python::object& module);
    //! @}
private:
    // some helper objects needed to initialize and run the Python interface
#if defined(FREEORION_MACOSX) || defined(FREEORION_WIN32) || defined(FREEORION_ANDROID)
    wchar_t* m_home_dir = nullptr;
    wchar_t* m_program_name = nullptr;
#endif
    // A copy of the systemExit exception to compare with returned
    // exceptions.  It can't be created in the exception handler.
    boost::python::object m_system_exit;
    boost::python::object m_traceback_format_exception;

    //! @name Modules finder and loader
    //! Finder and loader implementation properties
    //! @{
    std::function<void(boost::python::dict&)> m_populate_globals_func;
    std::filesystem::path m_modules_dir;
    boost::optional<boost::python::list> m_meta_path;
    int m_meta_path_len;
    //! @}
};

#endif /* defined(__FreeOrion__Util__PythonCommon__) */

