#include "Universe.h"
#include "System.h"

#include <boost/python.hpp>

using namespace boost::python;

void ExportUniverse()
{
    class_<Universe,boost::noncopyable>("Universe")

	// Accessors
	
	.def("Object",
	     (UniverseObject*(Universe::*)(int))
	     &Universe::Object,
	     return_internal_reference<>())
	.def("FindObjects",
	     (Universe::ObjectVec(Universe::*)(const UniverseObjectVisitor&))
	      &Universe::FindObjects)
	.def("__iter__", range(
		 (Universe::iterator (Universe::*)())&Universe::begin, 
		 (Universe::iterator (Universe::*)())&Universe::end))
	.def("LinearDistance",
	     (double(Universe::*)(System*,System*)const)
	     &Universe::LinearDistance)
	.def("LinearDistance",
	     (double(Universe::*)(int,int)const)
	     &Universe::LinearDistance)
	.def("ShortestPath",
	     (std::pair<std::list<System*>,double>(Universe::*)(System*,System*)const)
	     &Universe::ShortestPath)
	.def("ShortestPath",
	     (std::pair<std::list<System*>,double>(Universe::*)(int,int)const)
	     &Universe::ShortestPath)
	.def("LeastJumpsPath",
	     (std::pair<std::list<System*>,int>(Universe::*)(System*,System*)const)
	     &Universe::LeastJumpsPath)
	.def("LeastJumpsPath",
	     (std::pair<std::list<System*>,int>(Universe::*)(int,int)const)
	     &Universe::LeastJumpsPath)
	.def("ImmediateNeighbors",
	     (std::map<double,System*>(Universe::*)(System*)const)
	     &Universe::ImmediateNeighbors)
	.def("ImmediateNeighbors",
	     (std::map<double,System*>(Universe::*)(int)const)
	     &Universe::ImmediateNeighbors)
	
	// Member functions

	.def("GenerateObjectID",&Universe::GenerateObjectID)

	// Mutators

//	.def("SetUniverse",&Universe::SetUniverse)
	.def("Insert",&Universe::Insert)
	.def("InsertID",&Universe::InsertID)
	.def("CreateUniverse",&Universe::CreateUniverse)
	.def("ApplyEffects",&Universe::ApplyEffects)
	.def("Remove",&Universe::Remove,return_internal_reference<>())
	.def("Delete",&Universe::Delete)
	.def("EffectDestroy",&Universe::EffectDestroy)
	
	.add_property("UniverseWidth",&Universe::UniverseWidth)
// 	.add_property("InhibitUniverseObjectSignals",
// 		      make_function(&Universe::InhibitUniverseObjectSignals,
// 				    return_internal_reference<>()))
	     
	;

}
    
