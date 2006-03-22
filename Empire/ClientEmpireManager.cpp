#include "ClientEmpireManager.h"

#include "../util/MultiplayerCommon.h"
#include "../util/SitRepEntry.h"
#include "../util/XMLObjectFactory.h"

#include <map>
#include <list>



ClientEmpireManager::ClientEmpireManager() :
    EmpireManager()
{
}
   
bool ClientEmpireManager::HandleEmpireElementUpdate(const XMLElement& elem)
{   
    if (elem.Tag() != EmpireManager::EMPIRE_UPDATE_TAG)
        throw std::runtime_error("Attempted to construct a " + EmpireManager::EMPIRE_UPDATE_TAG + " from an XMLElement that had a tag other than \"" + EmpireManager::EMPIRE_UPDATE_TAG + "\"");

    RemoveAllEmpires();
    for (int i = 0; i < elem.NumChildren(); ++i) {
        Empire *decoded_empire = new Empire(elem.Child(i).Child(0));
        InsertEmpire(decoded_empire);
        decoded_empire->UpdateResourcePool();
        decoded_empire->UpdateResearchQueue();
    }

    return true;
}


XMLElement ClientEmpireManager::EncodeEmpires( )
{
    XMLElement this_turn(EmpireManager::EMPIRE_UPDATE_TAG);

    for (EmpireManager::iterator it = begin(); it != end(); it++) {
        XMLElement current_empire("Empire" + boost::lexical_cast<std::string>(it->second->EmpireID()));
        current_empire.AppendChild(it->second->XMLEncode());
        this_turn.AppendChild(current_empire);
    }

    return this_turn;
}
