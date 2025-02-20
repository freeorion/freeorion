#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"
#include "ConditionPythonParser.h"
#include "EffectPythonParser.h"
#include "EnumPythonParser.h"
#include "SourcePythonParser.h"

#include "../universe/Tech.h"
#include "../util/Directories.h"

#include <boost/python/import.hpp>
#include <boost/python/raw_function.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<UnlockableItem>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::set<std::string>&) { return os; }
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const Tech::TechInfo&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<TechCategory>>&) { return os; }
}
#endif

namespace {
    std::set<std::string>* g_categories_seen = nullptr;
    std::map<std::string, std::unique_ptr<TechCategory>, std::less<>>* g_categories = nullptr;

    void insert_category(std::map<std::string, std::unique_ptr<TechCategory>, std::less<>>& categories,
                         std::string& name, std::string& graphic, std::array<uint8_t, 4> color)
    {
        auto category_ptr = std::make_unique<TechCategory>(name, std::move(graphic), color);
        categories.emplace(std::move(name), std::move(category_ptr));
    }

    boost::python::object insert_category_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto graphic = boost::python::extract<std::string>(kw["graphic"])();
        auto colour = boost::python::extract<boost::python::tuple>(kw["colour"])();

        std::array<uint8_t, 4> color{0, 0, 0, 255};

        boost::python::stl_input_iterator<uint8_t> colour_begin(colour), colour_end;
        int colour_index = 0;
        for (auto it = colour_begin; it != colour_end; ++it) {
            if (colour_index < 4)
                color[colour_index] = *it;
            ++ colour_index;
        }

        insert_category(*g_categories, name, graphic, color);

        return boost::python::object();
    }

    struct py_grammar_category {
        boost::python::dict operator()(TechManager::TechContainer& techs) const {
            boost::python::dict globals(boost::python::import("builtins").attr("__dict__"));
            globals["Category"] = boost::python::raw_function(insert_category_);
            return globals;
        }
    };

    boost::python::object py_insert_tech_(TechManager::TechContainer& techs,
                                          const boost::python::tuple& args,
                                          const boost::python::dict& kw)
    {
        auto name = boost::python::extract<std::string>(kw["name"])();
        if (techs.contains(name)) {
            ErrorLogger() <<  "A tech already exists with name " << name;
            return boost::python::object();
        }

        auto description = boost::python::extract<std::string>(kw["description"])();
        auto short_description = boost::python::extract<std::string>(kw["short_description"])();
        auto category = boost::python::extract<std::string>(kw["category"])();

        std::unique_ptr<ValueRef::ValueRef<double>> researchcost;
        auto researchcost_args = boost::python::extract<value_ref_wrapper<double>>(kw["researchcost"]);
        if (researchcost_args.check()) {
            researchcost = ValueRef::CloneUnique(researchcost_args().value_ref);
        } else {
            researchcost = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["researchcost"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> researchturns;
        auto researchturns_args = boost::python::extract<value_ref_wrapper<int>>(kw["researchturns"]);
        if (researchturns_args.check()) {
            researchturns = ValueRef::CloneUnique(researchturns_args().value_ref);
        } else {
            researchturns = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["researchturns"])());
        }

        bool researchable = true;
        if (kw.has_key("researchable"))
            researchable = boost::python::extract<bool>(kw["researchable"])();

        std::set<std::string> tags;
        auto tags_args = boost::python::extract<boost::python::list>(kw["tags"])();
        boost::python::stl_input_iterator<std::string> tags_begin(tags_args), tags_end;
        for (auto it = tags_begin; it != tags_end; ++it)
            tags.insert(*it);

        std::vector<std::shared_ptr<Effect::EffectsGroup>> effectsgroups;
        if (kw.has_key("effectsgroups")) {
            boost::python::stl_input_iterator<effect_group_wrapper> effectsgroups_begin(kw["effectsgroups"]), effectsgroups_end;
            for (auto it = effectsgroups_begin; it != effectsgroups_end; ++it) {
                const auto& effects_group = *it->effects_group;
                effectsgroups.push_back(std::make_shared<Effect::EffectsGroup>(
                    ValueRef::CloneUnique(effects_group.Scope()),
                    ValueRef::CloneUnique(effects_group.Activation()),
                    ValueRef::CloneUnique(effects_group.Effects()),
                    effects_group.AccountingLabel(),
                    effects_group.StackingGroup(),
                    effects_group.Priority(),
                    effects_group.GetDescription(),
                    effects_group.TopLevelContent()
                ));
            }
        }

        std::set<std::string> prerequisites;
        if (kw.has_key("prerequisites")) {
            prerequisites.insert(boost::python::stl_input_iterator<std::string>(kw["prerequisites"]),
                boost::python::stl_input_iterator<std::string>());
        }
        if (std::count(prerequisites.begin(), prerequisites.end(), name)) {
            ErrorLogger() << "Tech " << name << " depends on itself!";
            return boost::python::object();
        }

        std::vector<UnlockableItem> unlock;
        if (kw.has_key("unlock")) {
            auto unlock_args = boost::python::extract<boost::python::list>(kw["unlock"]);
            if (unlock_args.check()) {
                boost::python::stl_input_iterator<unlockable_item_wrapper> unlock_begin(unlock_args()), unlock_end;
                for (auto it = unlock_begin; it != unlock_end; ++it)
                    unlock.push_back(it->item);
            } else {
                unlock.push_back(boost::python::extract<unlockable_item_wrapper>(kw["unlock"])().item);
            }
        }

        std::string graphic;
        if (kw.has_key("graphic"))
            graphic = boost::python::extract<std::string>(kw["graphic"])();


        g_categories_seen->emplace(category);

        auto name_copy{name};
        techs.emplace(std::piecewise_construct,
                      std::forward_as_tuple(std::move(name_copy)),
                      std::forward_as_tuple(std::move(name), std::move(description),
                                            std::move(short_description), std::move(category),
                                            std::move(researchcost), std::move(researchturns),
                                            researchable, std::move(tags), std::move(effectsgroups),
                                            std::move(prerequisites), std::move(unlock),
                                            std::move(graphic)));

        return boost::python::object();
    }

    struct py_grammar_techs {
        boost::python::dict globals;

        py_grammar_techs(const PythonParser& parser, TechManager::TechContainer& techs) :
            globals(boost::python::import("builtins").attr("__dict__"))
        {
#if PY_VERSION_HEX < 0x03080000
            globals["__builtins__"] = boost::python::import("builtins");
#endif
            RegisterGlobalsEffects(globals);
            RegisterGlobalsConditions(globals);
            RegisterGlobalsValueRefs(globals, parser);
            RegisterGlobalsSources(globals);
            RegisterGlobalsEnums(globals);

            globals["Tech"] = boost::python::raw_function(
                [&techs](const boost::python::tuple& args, const boost::python::dict& kw)
                { return py_insert_tech_(techs, args, kw); });
        }

        boost::python::dict operator()() const { return globals; }
    };
}

namespace parse {
    template <typename T>
    T techs(const PythonParser& parser, const boost::filesystem::path& path, bool& success) {
        TechManager::TechContainer techs_;
        std::map<std::string, std::unique_ptr<TechCategory>, std::less<>> categories;
        std::set<std::string> categories_seen;

        g_categories_seen = &categories_seen;
        g_categories = &categories;

        ScopedTimer timer("Techs Parsing");

        bool file_success = py_parse::detail::parse_file<py_grammar_category, TechManager::TechContainer>(
            parser, path / "Categories.inf.py", py_grammar_category(), techs_);

        py_grammar_techs p = py_grammar_techs(parser, techs_);

        for (const auto& file : ListDir(path, IsFOCPyScript))
            file_success = py_parse::detail::parse_file<py_grammar_techs>(parser, file, p) && file_success;

        TechManager::TechCategoryContainer cats;
        cats.reserve(categories.size());
        const auto extract_cats = [](auto& name_catptr) -> TechManager::TechCategoryContainer::value_type
        { return {name_catptr.first, std::move(*name_catptr.second)}; };

        std::transform(categories.begin(), categories.end(), std::inserter(cats, cats.end()), extract_cats);

        success = file_success;
        return std::make_tuple(std::move(techs_), std::move(cats), categories_seen);
    }
}

// explicitly instantiate techs.
// This allows Tech.h to only be included in this .cpp file and not Parse.h
// which recompiles all parsers if Tech.h changes.
template FO_PARSE_API TechManager::TechParseTuple parse::techs<TechManager::TechParseTuple>(
    const PythonParser& parser, const boost::filesystem::path& path, bool& success);
