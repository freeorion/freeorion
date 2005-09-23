#include "../Empire/EmpireManager.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace {

struct EmpireManagerWrap : EmpireManager, wrapper<EmpireManager>
{
    bool UpdateEmpireStatus(int e, std::string& n, GG::Clr c)
    {
	if (override o = this->get_override("UpdateEmpireStatus")) 
	    return o(e,n,c);
	return EmpireManager::UpdateEmpireStatus(e,n,c);
    }
    bool default_UpdateEmpireStatus(int e, std::string& n, GG::Clr c)
    {
	return this->EmpireManager::UpdateEmpireStatus(e,n,c);
    }
};

}

void ExportEmpireManager()
{
    class_<EmpireManagerWrap, boost::noncopyable>("EmpireManager")
	
	.def("InsertEmpire", &EmpireManager::InsertEmpire)
	.def("RemoveEmpire", &EmpireManager::RemoveEmpire)
	.def("RemoveAllEmpires", &EmpireManager::RemoveAllEmpires)

	.def("UpdateEmpireStatus", &EmpireManager::UpdateEmpireStatus, &EmpireManagerWrap::default_UpdateEmpireStatus)
	.def("Lookup",(Empire*(EmpireManager::*)(int))&EmpireManager::Lookup,
	     return_internal_reference<>())
	.def("__iter__",iterator<EmpireManager>())
	;
	
}
