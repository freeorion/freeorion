#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Diplomacy.h"
#include "../universe/Predicates.h"
#include "../universe/UniverseObject.h"
#include "../universe/Planet.h"

#include "../universe/Tech.h"

#include <GG/Clr.h>

#include <boost/function.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "PythonSetWrapper.h"
#include <boost/python/tuple.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/shared_ptr.hpp>

namespace {
    // Research queue tests whether it contains a Tech by name, but Python needs
    // a __contains__ function that takes a *Queue::Element.  This helper
    // functions take an Element and returns the associated Tech name string.
    const std::string&  TechFromResearchQueueElement(const ResearchQueue::Element& element)             { return element.name; }

    std::vector<std::string> (TechManager::*TechNamesVoid)(void) const =                                &TechManager::TechNames;
    boost::function<std::vector<std::string>(const TechManager*)> TechNamesMemberFunc =                 TechNamesVoid;

    std::vector<std::string> (TechManager::*TechNamesCategory)(const std::string&) const =              &TechManager::TechNames;
    boost::function<std::vector<std::string>(const TechManager*, const std::string&)>
                                                                  TechNamesCategoryMemberFunc =         TechNamesCategory;

    std::vector<std::string>    TechRecursivePrereqs(const Tech& tech, int empire_id)
    { return GetTechManager().RecursivePrereqs(tech.Name(), empire_id); }
    boost::function<std::vector<std::string>(const Tech& tech, int)> TechRecursivePrereqsFunc =         TechRecursivePrereqs;

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

    const SitRepEntry&  GetSitRep(const Empire& empire, int index) {
        static SitRepEntry EMPTY_ENTRY;
        if (index < 0 || index >= empire.NumSitRepEntries())
            return EMPTY_ENTRY;
        Empire::SitRepItr it = empire.SitRepBegin();
        std::advance(it, index);
        return *it;
    }
    boost::function<const SitRepEntry&(const Empire&, int)> GetEmpireSitRepFunc =                       GetSitRep;

    template<class T1, class T2>
    struct PairToTupleConverter {
        static PyObject* convert(const std::pair<T1, T2>& pair) {
            return boost::python::incref(boost::python::make_tuple(pair.first, pair.second).ptr());
        }
    };

    typedef PairToTupleConverter<int, int> myIntIntPairConverter;

    typedef std::pair<int, int> IntPair;

    typedef std::set<int> IntSet;

    typedef std::map<std::pair<int, int>,int > PairIntInt_IntMap;

    std::vector<IntPair>   obstructedStarlanesP(const Empire& empire) {
        std::set<IntPair > laneset;
        std::vector<IntPair>  retval;
        try {
            laneset = empire.SupplyObstructedStarlaneTraversals();
            for (std::set<std::pair<int, int> >::const_iterator it = laneset.begin(); it != laneset.end(); ++it)
            {retval.push_back(*it); }
            return retval;
        } catch (...) {
        }
        return retval;
    }
    boost::function<std::vector<IntPair>(const Empire&)> obstructedStarlanesFunc =      &obstructedStarlanesP;

    typedef std::pair<double, int> DoubleIntPair;

    typedef PairToTupleConverter<double, int> DoubleIntPairConverter;

    //boost::mpl::vector<DoubleIntPair, const Empire&, const ProductionQueue::Element&>()
    DoubleIntPair ProductionCostAndTimeP(const Empire& empire, const ProductionQueue::Element& element)
    { return empire.ProductionCostAndTime(element); }
    boost::function<DoubleIntPair(const Empire&,const ProductionQueue::Element& element )> ProductionCostAndTimeFunc = &ProductionCostAndTimeP;

    //.def("availablePP",                     make_function(&ProductionQueue::AvailablePP,          return_value_policy<return_by_value>()))
    //.add_property("allocatedPP",            make_function(&ProductionQueue::AllocatedPP,          return_internal_reference<>()))
    //.def("objectsWithWastedPP",             make_function(&ProductionQueue::ObjectsWithWastedPP,  return_value_policy<return_by_value>()))

    std::map<std::set<int>, double> PlanetsWithAvailablePP_P(const Empire& empire) {
        const boost::shared_ptr<ResourcePool>& industry_pool = empire.GetResourcePool(RE_INDUSTRY);
        const ProductionQueue& prodQueue = empire.GetProductionQueue();
        std::map<std::set<int>, double> planetsWithAvailablePP;
        std::map<std::set<int>, double> objectsWithAvailablePP = prodQueue.AvailablePP(industry_pool);
        for (std::map<std::set<int>, double>::iterator map_it = objectsWithAvailablePP.begin(); 
             map_it != objectsWithAvailablePP.end(); map_it++)
        {
            std::set<int> planetSet;
            std::set<int> objSet = map_it->first;
            for (std::set<int>::iterator obj_it = objSet.begin(); obj_it != objSet.end(); obj_it++) {
                TemporaryPtr<UniverseObject> location = GetUniverseObject(*obj_it);
                if (/* TemporaryPtr<const Planet> planet = */ universe_object_ptr_cast<const Planet>(location))
                    planetSet.insert(*obj_it);
            }
            if (!planetSet.empty())
                planetsWithAvailablePP[planetSet] = map_it->second;
        }
        return planetsWithAvailablePP;
    }
    boost::function<std::map<std::set<int>, double>(const Empire& )> PlanetsWithAvailablePP_Func = &PlanetsWithAvailablePP_P;

    std::map<std::set<int>, double> PlanetsWithAllocatedPP_P(const Empire& empire) {
        const ProductionQueue& prodQueue = empire.GetProductionQueue();
        std::map<std::set<int>, double> planetsWithAllocatedPP;
        std::map<std::set<int>, double> objectsWithAllocatedPP = prodQueue.AllocatedPP();
        for (std::map<std::set<int>, double>::iterator map_it = objectsWithAllocatedPP.begin(); 
             map_it != objectsWithAllocatedPP.end(); map_it++)
             {
                 std::set<int> planetSet;
                 std::set<int> objSet = map_it->first;
                 for (std::set<int>::iterator obj_it = objSet.begin(); obj_it != objSet.end(); obj_it++) {
                     TemporaryPtr<UniverseObject> location = GetUniverseObject(*obj_it);
                     if (/* TemporaryPtr<const Planet> planet = */ universe_object_ptr_cast<const Planet>(location))
                         planetSet.insert(*obj_it);
                 }
                 if (!planetSet.empty())
                     planetsWithAllocatedPP[planetSet] = map_it->second;
             }
             return planetsWithAllocatedPP;
    }
    boost::function<std::map<std::set<int>, double>(const Empire& )> PlanetsWithAllocatedPP_Func = &PlanetsWithAllocatedPP_P;

    std::set<std::set<int> > PlanetsWithWastedPP_P(const Empire& empire) {
        const boost::shared_ptr<ResourcePool>& industry_pool = empire.GetResourcePool(RE_INDUSTRY);
        const ProductionQueue& prodQueue = empire.GetProductionQueue();
        std::set<std::set<int> > planetsWithWastedPP;
        std::set<std::set<int> > objectsWithWastedPP = prodQueue.ObjectsWithWastedPP(industry_pool);
        for (std::set<std::set<int> >::iterator sets_it = objectsWithWastedPP.begin(); 
             sets_it != objectsWithWastedPP.end(); sets_it++)
             {
                 std::set<int> planetSet;
                 std::set<int> objSet = *sets_it;
                 for (std::set<int>::iterator obj_it = objSet.begin(); obj_it != objSet.end(); obj_it++) {
                     TemporaryPtr<UniverseObject> location = GetUniverseObject(*obj_it);
                     if (/* TemporaryPtr<const Planet> planet = */ universe_object_ptr_cast<const Planet>(location))
                         planetSet.insert(*obj_it);
                 }
                 if (!planetSet.empty())
                     planetsWithWastedPP.insert(planetSet);
             }
             return planetsWithWastedPP;
    }
    boost::function<std::set<std::set<int> >(const Empire& )> PlanetsWithWastedPP_Func = &PlanetsWithWastedPP_P;

}

namespace FreeOrionPython {
    using boost::python::class_;
    using boost::python::iterator;
    using boost::python::init;
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
        class_<PairIntInt_IntMap>("PairIntInt_IntMap")
            .def(boost::python::map_indexing_suite<PairIntInt_IntMap, true>())
        ;

        boost::python::to_python_converter<IntPair, myIntIntPairConverter>();

        class_<std::vector<IntPair> >("IntPairVec")
            .def(boost::python::vector_indexing_suite<std::vector<IntPair>, true>())
        ;
        class_<std::vector<ItemSpec> >("ItemSpecVec")
            .def(boost::python::vector_indexing_suite<std::vector<ItemSpec>, true>())
        ;
        boost::python::to_python_converter<DoubleIntPair, DoubleIntPairConverter>();

        class_<ResourcePool, boost::shared_ptr<ResourcePool>, boost::noncopyable >("resPool", boost::python::no_init);
        //FreeOrionPython::SetWrapper<int>::Wrap("IntSet");
        FreeOrionPython::SetWrapper<IntSet>::Wrap("IntSetSet");
        class_<std::map<std::set<int>, double> > ("resPoolMap")
            .def(boost::python::map_indexing_suite< std::map<std::set<int>, double>, true>() )
        ;
        
        ///////////////////
        //     Empire    //
        ///////////////////
        class_<Empire, noncopyable>("empire", no_init)
            .add_property("name",                   make_function(&Empire::Name,                    return_value_policy<copy_const_reference>()))
            .add_property("playerName",             make_function(&Empire::PlayerName,              return_value_policy<copy_const_reference>()))

            .add_property("empireID",               &Empire::EmpireID)
            .add_property("capitalID",              &Empire::CapitalID)

            .add_property("colour",                 make_function(&Empire::Color,                   return_value_policy<copy_const_reference>()))

            .def("buildingTypeAvailable",           &Empire::BuildingTypeAvailable)
            .add_property("availableBuildingTypes", make_function(&Empire::AvailableBuildingTypes,  return_internal_reference<>()))
            .def("shipDesignAvailable",             &Empire::ShipDesignAvailable)
            .add_property("allShipDesigns",         make_function(&Empire::ShipDesigns,             return_value_policy<return_by_value>()))
            .add_property("availableShipDesigns",   make_function(&Empire::AvailableShipDesigns,    return_value_policy<return_by_value>()))
            .add_property("productionQueue",        make_function(&Empire::GetProductionQueue,      return_internal_reference<>()))
            .def("productionCostAndTime",           make_function(
                                                        ProductionCostAndTimeFunc,      
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<DoubleIntPair, const Empire&, const ProductionQueue::Element& >()
                                                    ))
            .add_property("planetsWithAvailablePP", make_function(
                                                        PlanetsWithAvailablePP_Func,
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<std::map<std::set<int>, double>, const Empire& >()
                                                    ))
            .add_property("planetsWithAllocatedPP", make_function(
                                                        PlanetsWithAllocatedPP_Func,
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<std::map<std::set<int>, double>, const Empire& >()
                                                    ))
            .add_property("planetsWithWastedPP",    make_function(
                                                        PlanetsWithWastedPP_Func,
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<std::set<std::set<int> >, const Empire& >()
                                                    ))

            .def("techResearched",                  &Empire::TechResearched)
            .add_property("availableTechs",         make_function(&Empire::AvailableTechs,          return_internal_reference<>()))
            .def("getTechStatus",                   &Empire::GetTechStatus)
            .def("researchProgress",                &Empire::ResearchProgress)
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
            .def("getResourcePool",                 &Empire::GetResourcePool)

            .def("population",                      &Empire::Population)

            .add_property("fleetSupplyableSystemIDs",   make_function(&Empire::FleetSupplyableSystemIDs,    return_internal_reference<>()))
            .add_property("supplyUnobstructedSystems",  make_function(&Empire::SupplyUnobstructedSystems,   return_internal_reference<>()))

            .add_property("numSitReps",             &Empire::NumSitRepEntries)
            .def("getSitRep",                       make_function(
                                                        GetEmpireSitRepFunc,
                                                        return_internal_reference<>(),
                                                        boost::mpl::vector<const SitRepEntry&, const Empire&, int>()
                                                    ))
            //.add_property("obstructedStarlanes",  make_function(&Empire::SupplyObstructedStarlaneTraversals,   return_value_policy<return_by_value>()))   
            .def("obstructedStarlanes",             make_function(obstructedStarlanesFunc,
                                                    return_value_policy<return_by_value>(),
                                                    boost::mpl::vector<std::vector<IntPair>, const Empire&>()
            ))
        ;

        ////////////////////
        // Research Queue //
        ////////////////////
        class_<ResearchQueue::Element>("researchQueueElement", no_init)
            .def_readonly("tech",                   &ResearchQueue::Element::name)
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
            .def("__contains__",                    make_function(
                                                        boost::bind(InQueueFromResearchQueueElementFunc, _1, _2),
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<bool, const ResearchQueue*, const ResearchQueue::Element&>()
                                                    ))
            .add_property("totalSpent",             &ResearchQueue::TotalRPsSpent)
            .add_property("empireID",               &ResearchQueue::EmpireID)
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
            .add_property("locationID",             &ProductionQueue::Element::location)
            .add_property("allocation",             &ProductionQueue::Element::allocated_pp)
            .add_property("progress",               &ProductionQueue::Element::progress)
            .add_property("turnsLeft",              &ProductionQueue::Element::turns_left_to_completion)
            .add_property("remaining",              &ProductionQueue::Element::remaining)
            .add_property("blocksize",              &ProductionQueue::Element::blocksize)
            ;
        class_<ProductionQueue, noncopyable>("productionQueue", no_init)
            .def("__iter__",                        iterator<ProductionQueue>())  // ProductionQueue provides STL container-like interface to contained queue
            .def("__getitem__",                     ProductionQueueOperatorSquareBrackets,          return_internal_reference<>())
            .def("__len__",                         &ProductionQueue::size)
            .add_property("size",                   &ProductionQueue::size)
            .add_property("empty",                  &ProductionQueue::empty)
            .add_property("totalSpent",             &ProductionQueue::TotalPPsSpent)
            .add_property("empireID",               &ProductionQueue::EmpireID)
            .def("availablePP",                     make_function(&ProductionQueue::AvailablePP,          return_value_policy<return_by_value>()))
            .add_property("allocatedPP",            make_function(&ProductionQueue::AllocatedPP,          return_internal_reference<>()))
            .def("objectsWithWastedPP",             make_function(&ProductionQueue::ObjectsWithWastedPP,  return_value_policy<return_by_value>()))
            ;


        //////////////////
        //     Tech     //
        //////////////////
        class_<Tech, noncopyable>("tech", no_init)
            .add_property("name",                   make_function(&Tech::Name,              return_value_policy<copy_const_reference>()))
            .add_property("description",            make_function(&Tech::Description,       return_value_policy<copy_const_reference>()))
            .add_property("shortDescription",       make_function(&Tech::ShortDescription,  return_value_policy<copy_const_reference>()))
            .add_property("type",                   &Tech::Type)
            .add_property("category",               make_function(&Tech::Category,          return_value_policy<copy_const_reference>()))
            .def("researchCost",                    &Tech::ResearchCost)
            .def("perTurnCost",                     &Tech::PerTurnCost)
            .def("researchTime",                    &Tech::ResearchTime)
            .add_property("prerequisites",          make_function(&Tech::Prerequisites,     return_internal_reference<>()))
            .add_property("unlockedTechs",          make_function(&Tech::UnlockedTechs,     return_internal_reference<>()))
            .add_property("unlockedItems",          make_function(&Tech::UnlockedItems,     return_internal_reference<>()))
            .def("recursivePrerequisites",          make_function(
                                                        TechRecursivePrereqsFunc,
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<std::vector<std::string>, const Tech&, int>()
                                                    ))
        ;
        def("getTech",                              &GetTech,                               return_value_policy<reference_existing_object>());
        def("getTechCategories",                    &TechManager::CategoryNames,            return_value_policy<return_by_value>());
        def("techs",                                make_function(
                                                        boost::bind(TechNamesMemberFunc, &(GetTechManager())),
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<std::vector<std::string> >()
                                                    ));
        def("techsInCategory",                      make_function(
                                                        boost::bind(TechNamesCategoryMemberFunc, &(GetTechManager()), _1),
                                                        return_value_policy<return_by_value>(),
                                                        boost::mpl::vector<std::vector<std::string>, const std::string&>()
                                                    ));
        class_<ItemSpec>("itemSpec")
            .add_property("type",               &ItemSpec::type)
            .add_property("name",               &ItemSpec::name)
        ;

        ///////////////////
        //  SitRepEntry  //
        ///////////////////
        class_<SitRepEntry, noncopyable>("sitrep", no_init)
            .add_property("typeString",         make_function(&SitRepEntry::GetTemplateString,  return_value_policy<copy_const_reference>()))
            .def("getDataString",               make_function(&SitRepEntry::GetDataString,      return_value_policy<copy_const_reference>()))
            .def("getDataIDNumber",             &SitRepEntry::GetDataIDNumber)
            .add_property("getTags",            make_function(&SitRepEntry::GetVariableTags,    return_value_policy<return_by_value>()))
            .add_property("getTurn",            &SitRepEntry::GetTurn)
        ;

        ///////////////////////
        // DiplomaticMessage //
        ///////////////////////
        class_<DiplomaticMessage>("diplomaticMessage")
            .def(init<int, int, DiplomaticMessage::DiplomaticMessageType>())
            .add_property("type",               &DiplomaticMessage::GetType)
            .add_property("recipient",          &DiplomaticMessage::RecipientEmpireID)
            .add_property("sender",             &DiplomaticMessage::SenderEmpireID)
        ;

        ////////////////////////////
        // DiplomaticStatusUpdate //
        ////////////////////////////
        class_<DiplomaticStatusUpdateInfo>("diplomaticStatusUpdate")
            .def(init<int, int, DiplomaticStatus>())
            .add_property("status",             &DiplomaticStatusUpdateInfo::diplo_status)
            .add_property("empire1",            &DiplomaticStatusUpdateInfo::empire1_id)
            .add_property("empire2",            &DiplomaticStatusUpdateInfo::empire2_id)
        ;

        ///////////
        // Color //
        ///////////
        class_<GG::Clr>("GGColor", no_init)
            .add_property("r",                  &GG::Clr::r)
            .add_property("g",                  &GG::Clr::g)
            .add_property("b",                  &GG::Clr::b)
            .add_property("a",                  &GG::Clr::a)
        ;
    }
}
