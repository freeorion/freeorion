#include "client/AI/AIClientApp.h"

#include <boost/python.hpp>

using namespace boost::python;

namespace {

struct AIClientAppWrap : AIClientApp, wrapper<AIClientApp>
{
    AIClientAppWrap(int c,char**v)
	: AIClientApp(c,v) {}
    
    Message TurnOrdersMessage(bool s=false) const
	{
	    if (override o = this->get_override("TurnOrdersMessage"))
		return o(s);
	    return AIClientApp::TurnOrdersMessage(s);
	}
    Message default_TurnOrdersMessage(bool s=false) const
	{
	    return this->TurnOrdersMessage(s);
	}

    void StartTurn()
	{
	    if (override o = this->get_override("StartTurn"))
		o();
	    else
		AIClientApp::StartTurn();
	}
    void default_StartTurn()
	{
	    return this->StartTurn();
	}

    void HandleMessageImpl(const Message& m)
	{
	    if (override o = this->get_override("HandleMessageImpl"))
		o(m);
// 	    else
// 		AIClientApp::HandleMessageImpl(m);
	}
    void default_HandleMessageImpl(const Message& m)
	{
	    return this->HandleMessageImpl(m);
	}
    
    void HandleServerDisconnectImpl()
	{
	    if (override o = this->get_override("HandleServerDisconnectImpl"))
		o();
// 	    else
// 		AIClientApp::HandleServerDisconnectImpl();
	}
    void default_HandleServerDisconnectImpl()
	{
	    return this->HandleServerDisconnectImpl();
	}
};
}

void ExportAIClientApp()
{
    class_<AIClientAppWrap, bases<ClientApp>, boost::noncopyable>("AIClientApp",init<int,char**>())

#if 0
	// The following should be AIClientApp::HandleMessageImpl, but
	// since it's private, we have no choice...
	.def("HandleMessageImpl", &AIClientAppWrap::HandleMessageImpl, &AIClientAppWrap::default_HandleMessageImpl)
	.def("HandleServerDisconnectImpl",&AIClientAppWrap::HandleServerDisconnectImpl,&AIClientAppWrap::default_HandleServerDisconnectImpl)
	.def("TurnOrdersMessage",&AIClientAppWrap::TurnOrdersMessage,&AIClientAppWrap::default_TurnOrdersMessage)
	.def("StartTurn",&AIClientAppWrap::StartTurn,&AIClientAppWrap::default_StartTurn)
#endif	     
	;
}
