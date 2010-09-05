#include "../Empire/Empire.h"

#include <boost/function.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/python.hpp>

namespace {
    // Research queue tests whether it contains a Tech but Python needs a __contains__ function that takes a
    // *Queue::Element.  This helper functions take an Element and returns the associated Tech.
    const Tech*         TechFromResearchQueueElement(const ResearchQueue::Element& element) {
        return element.tech;
    }

    std::vector<std::string> (TechManager::*TechNamesVoid)(void) const =                                    &TechManager::TechNames;
    boost::function<std::vector<std::string>(const TechManager*)> TechNamesMemberFunc =                     TechNamesVoid;

    std::vector<std::string> (TechManager::*TechNamesCategory)(const std::string&) const =                  &TechManager::TechNames;
    boost::function<std::vector<std::string>(const TechManager*, const std::string&)>
                                                                  TechNamesCategoryMemberFunc =             TechNamesCategory;

    // Concatenate functions to create one that takes two parameters.  The first parameter is a ResearchQueue*, which
    // is passed directly to ResearchQueue::InQueue as the this pointer.  The second parameter is a
    // ResearchQueue::Element which is passed into TechFromResearchQueueElement, which returns a Tech*, which is
    // passed into ResearchQueue::InQueue as the second parameter.
    boost::function<bool(const ResearchQueue*, const ResearchQueue::Element&)> InQueueFromResearchQueueElementFunc =
        boost::bind(&ResearchQueue::InQueue, _1, boost::bind(TechFromResearchQueueElement, _2));

    // ProductionQueue::Element contains a ProductionItem which contains details of the item on the queue.  Need helper
    // functions to get the details about the item in the Element without adding extra pointless exposed classes to
    // the Python interface
    BuildType           BuildTypeFromProductionQueueElement(const ProductionQueue::Element& element)    { return element.item.build_type; }
    const std::string&  NameFromProductionQueueElement(const ProductionQueue::Element& element)         { return element.item.name; }
    int                 DesignIDFromProductionQueueElement(const ProductionQueue::Element& element)     { return element.item.design_id; }

    bool                (Empire::*BuildableItemBuilding)(BuildType, const std::string&, int) const =    &Empire::BuildableItem;
    bool                (Empire::*BuildableItemShip)(BuildType, int, int) const =                       &Empire::BuildableItem;

    const ProductionQueue::Element&
                            (ProductionQueue::*ProductionQueueOperatorSquareBrackets)(int) const =      &ProductionQueue::operator[];
}

namespace FreeOrionPython {
    using boost::python::class_;
    using boost::python::iterator;
    using boost::python::no_init;
    using boost::noncopyable;
    using boost::python::return_value_policy;
    using boost::python::copy_const_reference;
    using boost::python::reference_existing_object;
    using boost::python::return_by_value;
    using boost::python::return_internal_reference;

    /**
     * CallPolicies:
     *
     * return_value_policy<copy_const_reference>        when returning a relatively small object, such as a string,
     *                                                  that is returned by const reference or pointer
     *
     * return_value_policy<return_by_value>             when returning either a simple data type or a temporary object
     *                                                  in a function that will go out of scope after being returned
     *
     * return_internal_reference<>                      when returning an object or data that is a member of the object
     *                                                  on which the function is called (and shares its lifetime)
     *
     * return_value_policy<reference_existing_object>   when returning an object from a non-member function, or a
     *                                                  member function where the returned object's lifetime is not
     *                                                  fixed to the lifetime of the object on which the function is
     *                                                  called
     */
    void WrapEmpire() {
        ///////////////////
        //     Empire    //
        ///////////////////
        class_<Empire, noncopyable>("empire", no_init)
            .add_property("name",                   make_function(&Empire::Name,                    return_value_policy<copy_const_reference>()))
            .add_property("playerName",             make_function(&Empire::PlayerName,              return_value_policy<copy_const_reference>()))

            .add_property("empireID",               &Empire::EmpireID)
            .add_property("capitolID",              &Empire::CapitolID)

            .def("buildingTypeAvailable",           &Empire::BuildingTypeAvailable)
            .add_property("availableBuildingTypes", make_function(&Empire::AvailableBuildingTypes,  return_internal_reference<>()))
            .def("shipDesignAvailable",             &Empire::ShipDesignAvailable)
            .add_property("availableShipDesigns",   make_function(&Empire::AvailableShipDesigns,    return_value_policy<return_by_value>()))
            .add_property("productionQueue",        make_function(&Empire::GetProductionQueue,      return_internal_reference<>()))

            .def("techResearched",                  &Empire::TechResearched)
            .add_property("availableTechs",         make_function(&Empire::AvailableTechs,          return_internal_reference<>()))
            .def("getTechStatus",                   &Empire::GetTechStatus)
            .def("researchStatus",                  &Empire::ResearchStatus)
            .add_property("researchQueue",          make_function(&Empire::GetResearchQueue,        return_internal_reference<>()))

            .def("canBuild",                        BuildableItemBuilding)
            .def("canBuild",                        BuildableItemShip)

            .def("hasExploredSystem",               &Empire::HasExploredSystem)
            .add_property("exploredSystemIDs",      make_function(&Empire::ExploredSystems,         return_internal_reference<>()))

            .add_property("productionPoints",       make_function(&Empire::ProductionPoints,        return_value_policy<return_by_value>()))
            .def("resourceStockpile",               &Empire::ResourceStockpile)
            //.def("resourceMaxStockpile",            &Empire::ResourceMaxStockpile)
            .def("resourceProduction",              &Empire::ResourceProduction)
            .def("resourceAvailable",               &Empire::ResourceAvailable)

            .def("population",                      &Empire::Population)

            .add_property("fleetSupplyableSystemIDs",   make_function(&Empire::FleetSupplyableSystemIDs,return_internal_reference<>()))
            .add_property("supplyUnobstructedSystems",  make_function(&Empire::SupplyUnobstructedSystems,return_internal_reference<>()))
        ;


        ////////////////////
        // Research Queue //
        ////////////////////
        class_<ResearchQueue::Element>("researchQueueElement", no_init)
            .add_property("tech",                   make_getter(&ResearchQueue::Element::tech,      return_value_policy<reference_existing_object>()))
            .def_readonly("allocation",             &ResearchQueue::Element::allocated_rp)
            .def_readonly("turnsLeft",              &ResearchQueue::Element::turns_left)
        ;
        class_<ResearchQueue, noncopyable>("researchQueue", no_init)
            .def("__iter__",                        iterator<ResearchQueue>())  // ResearchQueue provides STL container-like interface to contained queue
            .def("__getitem__",                     &ResearchQueue::operator[],                     return_internal_reference<>())
            .def("__len__",                         &ResearchQueue::size)
            .add_property("size",                   &ResearchQueue::size)
            .add_property("empty",                  &ResearchQueue::empty)
            .def("inQueue",                         &ResearchQueue::InQueue)
            .def("inQueue",                         make_function(
                                                        boost::bind(&ResearchQueue::InQueue, _1, boost::bind(&GetTech, _2)),
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<bool, const ResearchQueue*, const std::string&>()
                                                    ))
            .def("__contains__",                    make_function(
                                                        boost::bind(InQueueFromResearchQueueElementFunc, _1, _2),
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<bool, const ResearchQueue*, const ResearchQueue::Element&>()
                                                    ))
            .add_property("totalSpent",             &ResearchQueue::TotalRPsSpent)
        ;

        //////////////////////
        // Production Queue //
        //////////////////////
        class_<ProductionQueue::Element>("productionQueueElement", no_init)
            .add_property("name",                   make_function(
                                                        boost::bind(NameFromProductionQueueElement, _1),
                                                        return_value_policy<copy_const_reference>(),
                                                        boost::mpl::vector<const std::string&, const ProductionQueue::Element&>()
                                                    ))
            .add_property("designID",               make_function(
                                                        boost::bind(DesignIDFromProductionQueueElement, _1),
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<int, const ProductionQueue::Element&>()
                                                    ))
            .add_property("buildType",              make_function(
                                                        boost::bind(BuildTypeFromProductionQueueElement, _1),
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<BuildType, const ProductionQueue::Element&>()
                                                    ))
            .def_readonly("locationID",             &ProductionQueue::Element::location)
            .def_readonly("allocation",             &ProductionQueue::Element::allocated_pp)
            .def_readonly("turnsLeft",              &ProductionQueue::Element::turns_left_to_completion)
        ;
        class_<ProductionQueue, noncopyable>("productionQueue", no_init)
            .def("__iter__",                        iterator<ProductionQueue>())  // ProductionQueue provides STL container-like interface to contained queue
            .def("__getitem__",                     ProductionQueueOperatorSquareBrackets,          return_internal_reference<>())
            .def("__len__",                         &ProductionQueue::size)
            .add_property("size",                   &ProductionQueue::size)
            .add_property("empty",                  &ProductionQueue::empty)
            .add_property("totalSpent",             &ProductionQueue::TotalPPsSpent)
        ;


        //////////////////
        //     Tech     //
        //////////////////
        class_<Tech, noncopyable>("tech", no_init)
            .add_property("name",               make_function(&Tech::Name,              return_value_policy<copy_const_reference>()))
            .add_property("description",        make_function(&Tech::Description,       return_value_policy<copy_const_reference>()))
            .add_property("shortDescription",   make_function(&Tech::ShortDescription,  return_value_policy<copy_const_reference>()))
            .add_property("type",               &Tech::Type)
            .add_property("category",           make_function(&Tech::Category,          return_value_policy<copy_const_reference>()))
            .add_property("researchCost",       &Tech::ResearchCost)
            .add_property("researchTime",       &Tech::ResearchTime)
            .add_property("prerequisites",      make_function(&Tech::Prerequisites,     return_internal_reference<>()))
            .add_property("unlockedTechs",      make_function(&Tech::UnlockedTechs,     return_internal_reference<>()))
        ;
        def("getTech",                          &GetTech,                               return_value_policy<reference_existing_object>());
        def("getTechCategories",                &TechManager::CategoryNames,            return_value_policy<return_by_value>());
        def("techs",                            make_function(
                                                    boost::bind(TechNamesMemberFunc, &(GetTechManager())),
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<std::string> >()
                                                ));
        def("techsInCategory",                  make_function(
                                                    boost::bind(TechNamesCategoryMemberFunc, &(GetTechManager()), _1),
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<std::string>, const std::string&>()
                                                ));
    }
}
