#include "Tech.h"

#include <boost/python.hpp>

using namespace boost::python;

void ExportTechManager()
{
    class_<TechManager>("TechManager",no_init)
	.def("GetTech", &TechManager::GetTech, return_internal_reference<>())
	.add_property("CategoryNames", make_function(&TechManager::CategoryNames,
						     return_internal_reference<>()))
	.def("AllNextTechs", &TechManager::AllNextTechs)
	.def("CheapestNextTech", &TechManager::CheapestNextTech,
	     return_internal_reference<>())
	.def("NextTechsTowards", &TechManager::NextTechsTowards)
	.def("CheapestNextTechTowards", &TechManager::CheapestNextTechTowards,
	     return_internal_reference<>())
	.def("__iter__", range<return_internal_reference<> >(
		 &TechManager::begin,
		 &TechManager::end))
// 	.def("Categories", range(&TechManager::category_begin,
// 				 &TechManager::category_end))
	.def("GetTechManager", &TechManager::GetTechManager,
	     return_value_policy<copy_non_const_reference>())
	.staticmethod("GetTechManager")
	;
}
