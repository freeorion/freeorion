#include "ClientApp.h"

#include "../Empire/TechManager.h"
#include "../network/XDiff.hpp"

#include <stdexcept>


namespace {
    bool temp_header_bool = RecordHeaderFile(ClientAppRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


// static member(s)
ClientApp* ClientApp::s_app = 0;

ClientApp::ClientApp() : 
    m_multiplayer_lobby_wnd(0),
    m_current_combat(0), 
    m_player_id(-1),
    m_empire_id(-1)
{
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of ClientApp");
   
    s_app = this;

    // initialize tech manager
    TechManager::instance().LoadTechTree( "" );

}

ClientApp::~ClientApp()
{
    // shutdown tech tree
    TechManager::instance().ClearAll();
}

Message ClientApp::TurnOrdersMessage(bool save_game_data/* = false*/) const
{
    GG::XMLDoc orders_doc;
    if (save_game_data)
        orders_doc.root_node.AppendChild("save_game_data");
    orders_doc.root_node.AppendChild(GG::XMLElement("Orders"));
    for (OrderSet::const_iterator order_it = m_orders.begin(); order_it != m_orders.end(); ++order_it) {
        orders_doc.root_node.LastChild().AppendChild(order_it->second->XMLEncode());
    }
    return ::TurnOrdersMessage(m_player_id, -1, orders_doc);
}

void ClientApp::HandleMessage(const Message& msg)
{
    s_app->HandleMessageImpl(msg);
}

void ClientApp::HandleServerDisconnect()
{
    s_app->HandleServerDisconnectImpl();
}

Universe& ClientApp::GetUniverse()
{
    return s_app->m_universe;
}

MultiplayerLobbyWnd* ClientApp::MultiplayerLobby()
{
    return s_app->m_multiplayer_lobby_wnd;
}

ClientEmpireManager& ClientApp::Empires()
{
    return s_app->m_empires;
}

CombatModule* ClientApp::CurrentCombat()
{
    return s_app->m_current_combat;
}

OrderSet& ClientApp::Orders()
{
    return s_app->m_orders;
}

ClientNetworkCore& ClientApp::NetworkCore()
{
    return s_app->m_network_core;
}


void ClientApp::StartTurn( )
{
    // send message
    m_network_core.SendMessage(TurnOrdersMessage());

    // clear order set
    m_orders.Reset( );
}


int ClientApp::GetNewObjectID( )
{
    int new_id = UniverseObject::INVALID_OBJECT_ID; 

    // ask the server for a new universe object ID. This is a blocking method and can timeout without a valid ID
    Message msg;
    s_app->m_network_core.SendSynchronousMessage( RequestNewObjectIDMessage( s_app->m_player_id ), msg );

    if (msg.GetText().length()>0)
      new_id = boost::lexical_cast<int>(msg.GetText());

    return new_id;
}


void ClientApp::UpdateTurnData( const GG::XMLDoc &diff )
{
    GG::XMLDoc new_doc;

    // we may not have a universe object if nothing has changed 
    if (diff.root_node.ContainsChild("Universe")) 
    {
        // get old universe data
        new_doc.root_node.AppendChild(m_universe.XMLEncode(m_empire_id));
    }
 
    if (diff.root_node.ContainsChild(EmpireManager::EMPIRE_UPDATE_TAG))
    {
        // get old empire data
        new_doc.root_node.AppendChild(m_empires.EncodeEmpires());
    }

    // XPatch
    XPatch( new_doc, diff );

#if 0
    // write patch data for debug
    std::ofstream output("patch_merge_result.txt");
    new_doc.WriteDoc(output);
    output.close();
#endif

    // apply universe
    if (new_doc.root_node.ContainsChild( "Universe" ) )
    {
        m_universe.SetUniverse( new_doc.root_node.Child( "Universe" ) );
    }

    // apply empire
    if (new_doc.root_node.ContainsChild( EmpireManager::EMPIRE_UPDATE_TAG ) ) 
    {
        m_empires.HandleEmpireElementUpdate( new_doc.root_node.Child( EmpireManager::EMPIRE_UPDATE_TAG ) );
    }
}
