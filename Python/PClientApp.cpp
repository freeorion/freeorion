#include "client/ClientApp.h"
//#include "UI/MultiplayerLobbyWnd.h"
#include <boost/python.hpp>

using namespace boost::python;

namespace {

struct ClientAppWrap : ClientApp, wrapper<ClientApp>
{
//     public:
//     virtual void HandleMessageImpl(const Message& msg) = 0;
//     virtual void HandleServerDisconnectImpl() {}

    Message TurnOrdersMessage(bool s=false) const
	{
	    if (override o = this->get_override("TurnOrdersMessage"))
		return o(s);
	    return ClientApp::TurnOrdersMessage(s);
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
		ClientApp::StartTurn();
	}
    void default_StartTurn()
	{
	    return this->StartTurn();
	}

    void HandleMessageImpl(const Message& m)
	{
	    this->get_override("HandleMessageImpl")(m);
	}
    void HandleServerDisconnectImpl()
	{
	    this->get_override("HandleServerDisconnectImpl")();
	}
    
    
};
}

void ExportClientApp()
{
    class_<ClientAppWrap, boost::noncopyable>("ClientApp")

	// Accessors
	.add_property("PlayerName",make_function(&ClientApp::PlayerName,
						 return_internal_reference<>()))
	.add_property("PlayerID",&ClientApp::PlayerID)
	.add_property("EmpireID",&ClientApp::EmpireID)

	// Static members

	.def("HandleMessage",&ClientApp::HandleMessage)
	.def("HandleServerDisconnect",&ClientApp::HandleServerDisconnect)
	.def("GetNewObjectID",&ClientApp::GetNewObjectID)
// 	.def("MultiplayerLobby",&ClientApp::MultiplayerLobby,
// 	     return_internal_reference<>())

	.staticmethod("HandleMessage")
	.staticmethod("HandleServerDisconnect")
	.staticmethod("GetNewObjectID")
	.staticmethod("MultiplayerLobby")
	.add_property("Universe",make_function(&ClientApp::GetUniverse,
					       return_internal_reference<>()))
	.add_property("Empires",
		      make_function(&ClientApp::Empires,
				    return_internal_reference<>()))
// 	.add_property("CurrentCombat",
// 		      make_function(&ClientApp::CurrentCombat,
// 				    return_internal_reference<>()))
	.add_property("Orders",make_function(&ClientApp::Orders,
					     return_internal_reference<>()))
	.add_property("NetworkCore",make_function(&ClientApp::NetworkCore,
						  return_internal_reference<>()))

	// virtual functions

	// The following should be ClientApp::HandleMessageImpl, but
	// since it's private, we have no choice...
	.def("HandleMessageImpl",pure_virtual(&ClientAppWrap::HandleMessageImpl))
	.def("HandleServerDisconnectImpl",pure_virtual(&ClientAppWrap::HandleServerDisconnectImpl))
	.def("TurnOrdersMessage",&ClientApp::TurnOrdersMessage,&ClientAppWrap::default_TurnOrdersMessage)
	.def("StartTurn",&ClientApp::StartTurn,&ClientAppWrap::default_StartTurn)
	     
	;
}
