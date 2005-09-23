#include "../Empire/ClientEmpireManager.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace {

struct ClientEmpireManagerWrap : ClientEmpireManager, wrapper<ClientEmpireManager>
{
    bool UpdateEmpireStatus(int e, std::string& n, GG::Clr c)
    {
	if (override o = this->get_override("UpdateEmpireStatus")) 
	    return o(e,n,c);
	return ClientEmpireManager::UpdateEmpireStatus(e,n,c);
    }
    bool default_UpdateEmpireStatus(int e, std::string& n, GG::Clr c)
    {
	return this->ClientEmpireManager::UpdateEmpireStatus(e,n,c);
    }
};

}

void ExportClientEmpireManager()
{
    class_<ClientEmpireManagerWrap, bases<EmpireManager>, boost::noncopyable>("ClientEmpireManager")
	

	.def("UpdateEmpireStatus", &ClientEmpireManager::UpdateEmpireStatus, &ClientEmpireManagerWrap::default_UpdateEmpireStatus)
	;
	
}
