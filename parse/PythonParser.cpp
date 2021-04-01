#include "PythonParser.h"

#include "../universe/UnlockableItem.h"
#include "../universe/ValueRef.h"
#include "../universe/ValueRefs.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/PythonCommon.h"
#include "Parse.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include <boost/algorithm/string.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/core/ref.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/python/class.hpp>
#include <boost/python/import.hpp>
#include <boost/python/object.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/operators.hpp>
#include <memory>

namespace py = boost::python;

namespace {
    struct import_error : std::runtime_error {
        import_error(const std::string& what) : std::runtime_error(what) {}
    };

    void translate(import_error const& e) {
        PyErr_SetString(PyExc_ImportError, e.what());
    }
}

struct module_spec {
    module_spec(const std::string& name, const std::string& parent_, const PythonParser& parser_)
        : fullname(name)
        , parent(parent_)
        , parser(parser_)
    { }

    py::list path;
    std::string fullname;
    std::string parent;
    const PythonParser& parser;
};

PythonParser::PythonParser(PythonCommon& _python, const boost::filesystem::path& scripting_dir)
    : m_python(_python)
    , m_scripting_dir(scripting_dir)
{
    if (!m_python.IsPythonRunning()) {
        ErrorLogger() << "Python parse given non-initialized python!";
        throw std::runtime_error("Python isn't initialized");
    }

    try {
        type_int = py::import("builtins").attr("int");
        type_float = py::import("builtins").attr("float");
        type_bool = py::import("builtins").attr("bool");
        type_str = py::import("builtins").attr("str");

        py::register_exception_translator<import_error>(&translate);

        py::class_<PythonParser, py::bases<>, PythonParser, boost::noncopyable>("PythonParser", py::no_init)
            .def("find_spec", &PythonParser::find_spec)
            .def("create_module", &PythonParser::create_module)
            .def("exec_module", &PythonParser::exec_module);

        py::class_<module_spec>("PythonParserSpec", py::no_init)
            .def_readonly("name", &module_spec::fullname)
            .add_static_property("loader", py::make_getter(*this, py::return_value_policy<py::reference_existing_object>()))
            .def_readonly("submodule_search_locations", &module_spec::path)
            .def_readonly("has_location", false)
            .def_readonly("cached", false)
            .def_readonly("parent", &module_spec::parent);

        // Use wrappers to not collide with types in server and AI
        py::class_<value_ref_wrapper<int>>("ValueRefInt", py::no_init)
            .def(py::self_ns::self < py::self_ns::self)
            .def(py::self_ns::self == py::self_ns::self);
        py::class_<value_ref_wrapper<double>>("ValueRefDouble", py::no_init)
            .def("__call__", &value_ref_wrapper<double>::call)
            .def(int() * py::self_ns::self)
            .def(py::self_ns::self + int())
            .def(py::self_ns::self + double())
            .def(py::self_ns::self + py::self_ns::self)
            .def(py::self_ns::self - py::self_ns::self);
        py::class_<value_ref_wrapper<std::string>>("ValueRefString", py::no_init);
        py::class_<condition_wrapper>("Condition", py::no_init)
            .def(py::self_ns::self & py::self_ns::self)
            .def(py::self_ns::self | py::self_ns::self)
            .def(~py::self_ns::self);
        py::class_<effect_wrapper>("Effect", py::no_init);
        py::class_<effect_group_wrapper>("EffectsGroup", py::no_init);
        py::class_<enum_wrapper<UnlockableItemType>>("UnlockableItemType", py::no_init);
        py::class_<enum_wrapper<EmpireAffiliationType>>("EmpireAffiliationType", py::no_init);
        py::class_<unlockable_item_wrapper>("UnlockableItem", py::no_init);
        py::class_<source_wrapper>("__Source", py::no_init)
            .def_readonly("Owner", &source_wrapper::owner);
        py::class_<target_wrapper>("__Target", py::no_init)
            .def_readonly("HabitableSize", &target_wrapper::habitable_size)
            .def_readonly("MaxShield", &target_wrapper::max_shield)
            .def_readonly("MaxDefense", &target_wrapper::max_defense)
            .def_readonly("MaxTroops", &target_wrapper::max_troops)
            .def_readonly("ID", &target_wrapper::id)
            .def_readonly("Owner", &target_wrapper::owner)
            .def_readonly("SystemID", &target_wrapper::system_id)
            .def_readonly("DesignID", &target_wrapper::design_id);
        py::class_<local_candidate_wrapper>("__LocalCandidate", py::no_init)
            .def_readonly("LastTurnAttackedByShip", &local_candidate_wrapper::last_turn_attacked_by_ship)
            .def_readonly("LastTurnConquered", &local_candidate_wrapper::last_turn_conquered)
            .def_readonly("LastTurnColonized", &local_candidate_wrapper::last_turn_colonized);

        m_meta_path = py::extract<py::list>(py::import("sys").attr("meta_path"));
        m_meta_path.append(boost::cref(*this));
    } catch (const boost::python::error_already_set& err) {
        m_python.HandleErrorAlreadySet();
        if (!m_python.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (m_python.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
            }
        }
        throw std::runtime_error("Python parser failed to initialize");
    }
}

PythonParser::~PythonParser() {
    m_meta_path.pop(py::len(m_meta_path) - 1);
    // ToDo: ensure type of removed parser
    // ToDo: clean up sys.modules
}

bool PythonParser::ParseFileCommon(const boost::filesystem::path& path,
                                   const boost::python::dict& globals,
                                   std::string& filename, std::string& file_contents) const
{
    filename = path.string();

    bool read_success = ReadFile(path, file_contents);
    if (!read_success) {
        ErrorLogger() << "Unable to open data file " << filename;
        return false;
    }

    try {
        m_current_globals = globals;
        py::exec(file_contents.c_str(), globals);
        m_current_globals = boost::none;
    } catch (const boost::python::error_already_set& err) {
        m_current_globals = boost::none;
        m_python.HandleErrorAlreadySet();
        if (!m_python.IsPythonRunning()) {
            ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
            if (m_python.Initialize()) {
                ErrorLogger() << "Python interpreter successfully restarted.";
            } else {
                ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
            }
        }
        return false;
    } catch (...) {
        m_current_globals = boost::none;
        throw;
    }

    return true;
}

py::object PythonParser::find_spec(const std::string& fullname, const py::object& path, const py::object& target) const {
    auto module_path(m_scripting_dir);
    std::string parent = "";
    std::string current = "";
    for (boost::algorithm::split_iterator<std::string::const_iterator> it
         = boost::algorithm::make_split_iterator(fullname, boost::algorithm::token_finder(boost::algorithm::is_any_of(".")));
         it != boost::algorithm::split_iterator<std::string::const_iterator>();
         ++ it)
    {
        module_path = module_path / boost::copy_range<std::string>(*it);
        if (!current.empty()) {
            if (parent.empty()) {
                parent = std::move(current);
            } else {
                parent = parent + "." + current;
            }
        }
        current = boost::copy_range<std::string>(*it);
    }

    if (boost::filesystem::exists(module_path) && boost::filesystem::is_directory(module_path)) {
        return py::object(module_spec(fullname, parent, *this));
    } else {
        module_path.replace_extension("py");
        if (boost::filesystem::exists(module_path) && boost::filesystem::is_regular_file(module_path)) {
            return py::object(module_spec(fullname, parent, *this));
        } else {
            return py::object();
        }
    }
}

py::object PythonParser::create_module(const module_spec& spec) {
    return py::object();
}

py::object PythonParser::exec_module(py::object& module) {
    std::string fullname = py::extract<std::string>(module.attr("__name__"));

    py::dict m_dict = py::extract<py::dict>(module.attr("__dict__"));

    auto module_path(m_scripting_dir);
    for (boost::algorithm::split_iterator<std::string::iterator> it
         = boost::algorithm::make_split_iterator(fullname, boost::algorithm::token_finder(boost::algorithm::is_any_of(".")));
         it != boost::algorithm::split_iterator<std::string::iterator>();
         ++ it)
    {
        module_path = module_path / boost::copy_range<std::string>(*it);
    }

    if (boost::filesystem::exists(module_path) && boost::filesystem::is_directory(module_path)) {
        return py::object();
    } else {
        module_path.replace_extension("py");
        if (boost::filesystem::exists(module_path) && boost::filesystem::is_regular_file(module_path)) {
            std::string file_contents;
            bool read_success = ReadFile(module_path, file_contents);
            if (!read_success) {
                ErrorLogger() << "Unable to open data file " << module_path.string();
                throw import_error("Unreadable module");
            }

            // store globals content in module namespace
            // it is required so functions in the same module will see each other
            // and still import will work
            py::dict globals = m_current_globals ? (*m_current_globals) : py::dict();
            py::stl_input_iterator<py::object> g_begin(globals.keys()), g_end;
            for (auto it = g_begin; it != g_end; ++ it) {
                if (!m_dict.has_key(*it)) {
                    m_dict[*it] = globals[*it];
                }
            }

            try {
                py::exec(file_contents.c_str(),
                         m_dict,
                         m_dict);
            } catch (const boost::python::error_already_set& err) {
                m_python.HandleErrorAlreadySet();
                if (!m_python.IsPythonRunning()) {
                    ErrorLogger() << "Python interpreter is no longer running.  Attempting to restart.";
                    if (m_python.Initialize()) {
                        ErrorLogger() << "Python interpreter successfully restarted.";
                    } else {
                        ErrorLogger() << "Python interpreter failed to restart.  Exiting.";
                    }
                }
                throw import_error("Cannot execute module");
            }

            return py::object();
        } else {
            throw import_error("Module not existed");
        }
    }
}

