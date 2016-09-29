#include "AIWrapper.h"

#include "../../AI/AIInterface.h"
#include "../../universe/Universe.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"
#include "../../util/i18n.h"
#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../Empire/Empire.h"
#include "../../Empire/EmpireManager.h"
#include "../../Empire/Diplomacy.h"
#include "../SetWrapper.h"
#include "../CommonWrappers.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/scope.hpp>

using boost::python::class_;
using boost::python::def;
using boost::python::return_value_policy;
using boost::python::copy_const_reference;
using boost::python::reference_existing_object;
using boost::python::return_by_value;
using boost::python::return_internal_reference;

using boost::python::vector_indexing_suite;
using boost::python::map_indexing_suite;

using boost::python::object;
using boost::python::import;
using boost::python::error_already_set;
using boost::python::exec;
using boost::python::dict;
using boost::python::list;
using boost::python::extract;


////////////////////////
// Python AIInterface //
////////////////////////
// disambiguation of overloaded functions
const std::string&  (*AIIntPlayerNameVoid)(void) =              &AIInterface::PlayerName;
const std::string&  (*AIIntPlayerNameInt)(int) =                &AIInterface::PlayerName;

const Empire*       (*AIIntGetEmpireVoid)(void) =               &AIInterface::GetEmpire;
const Empire*       (*AIIntGetEmpireInt)(int) =                 &AIInterface::GetEmpire;

int                 (*AIIntNewFleet)(const std::string&, int) = &AIInterface::IssueNewFleetOrder;
int                 (*AIIntScrap)(int)                        = &AIInterface::IssueScrapOrder;



namespace {
    // static string to save AI state
    static std::string s_save_state_string("");

    int IssueCreateShipDesignOrderWrapper(const std::string& name, const std::string& description,
                                          const std::string& hull, boost::python::list partsList,
                                          const std::string& icon, const std::string& model, bool nameDescInStringTable)
    {
        std::vector<std::string> parts;
        int const numParts = boost::python::len(partsList);
        for (int i = 0; i < numParts; i++)
            parts.push_back(boost::python::extract<std::string>(partsList[i]));
        int result = AIInterface::IssueCreateShipDesignOrder(name, description, hull, parts, icon, model, nameDescInStringTable);
        return result;
    }

    boost::python::list GetUserStringList(const std::string& list_key) {
        std::list<std::string> retval;
        UserStringList(list_key, retval);
        boost::python::list ret_list;
        for (std::list< std::string >::iterator it = retval.begin(); it != retval.end(); ++it)
            ret_list.append(*it);
        return ret_list;
    }

    boost::python::str GetUserConfigDirWrapper()
    { return boost::python::str(PathString(GetUserConfigDir())); }

    boost::python::str GetUserDataDirWrapper()
    { return boost::python::str(PathString(GetUserDataDir())); }

}

namespace FreeOrionPython {
    const std::string& GetStaticSaveStateString()
    { return s_save_state_string; }

    void SetStaticSaveStateString(const std::string& new_state_string)
    { s_save_state_string = new_state_string; }

    void ClearStaticSaveStateString()
    { s_save_state_string = ""; }

    /** Expose AIInterface to Python.
     *
     * CallPolicies:
     *
     * return_value_policy<copy_const_reference>        when returning a relatively small object, such as a string,
     *                                                  that is returned by const reference or pointer
     *
     * return_value_policy<return_by_value>             when returning either a simple data type or a temporary object
     *                                                  in a function that will go out of scope after being returned
     *
     * return_value_policy<reference_existing_object>   when returning an object from a non-member function
     */
    void WrapAI()
    {
        def("playerName",               AIIntPlayerNameVoid,            return_value_policy<copy_const_reference>(), "Returns the name (string) of this AI player.");
        def("playerName",               AIIntPlayerNameInt,             return_value_policy<copy_const_reference>(), "Returns the name (string) of the player with the indicated playerID (int).");

        def("playerID",                 AIInterface::PlayerID, "Returns the integer id of this AI player.");
        def("empirePlayerID",           AIInterface::EmpirePlayerID, "Returns the player ID (int) of the player who is controlling the empire with the indicated empireID (int).");
        def("allPlayerIDs",             AIInterface::AllPlayerIDs,      return_value_policy<return_by_value>(), "Returns an object (intVec) that contains the player IDs of all players in the game.");

        def("playerIsAI",               AIInterface::PlayerIsAI, "Returns True (boolean) if the player with the indicated playerID (int) is controlled by an AI and false (boolean) otherwise.");
        def("playerIsHost",             AIInterface::PlayerIsHost, "Returns True (boolean) if the player with the indicated playerID (int) is the host player for the game and false (boolean) otherwise.");

        def("empireID",                 AIInterface::EmpireID, "Returns the empire ID (int) of this AI player's empire.");
        def("playerEmpireID",           AIInterface::PlayerEmpireID, "Returns the empire ID (int) of the player with the specified player ID (int).");
        def("allEmpireIDs",             AIInterface::AllEmpireIDs,      return_value_policy<return_by_value>(), "Returns an object (intVec) that contains the empire IDs of all empires in the game.");

        def("getEmpire",                AIIntGetEmpireVoid,             return_value_policy<reference_existing_object>(), "Returns the empire object (Empire) of this AI player");
        def("getEmpire",                AIIntGetEmpireInt,              return_value_policy<reference_existing_object>(), "Returns the empire object (Empire) with the specified empire ID (int)");

        def("getUniverse",              AIInterface::GetUniverse,       return_value_policy<reference_existing_object>(), "Returns the universe object (Universe)");

        def("currentTurn",              AIInterface::CurrentTurn, "Returns the current game turn (int).");

        def("getAIConfigStr",           AIInterface::GetAIConfigStr,    return_value_policy<return_by_value>());
        def("getAIDir",                 AIInterface::GetAIDir,          return_value_policy<return_by_value>());
        def("getUserConfigDir",         GetUserConfigDirWrapper,        /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific configuration.");
        def("getUserDataDir",           GetUserDataDirWrapper,          /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific data (saves, etc.).");

        def("initMeterEstimatesDiscrepancies",      AIInterface::InitMeterEstimatesAndDiscrepancies);
        def("updateMeterEstimates",                 AIInterface::UpdateMeterEstimates);
        def("updateResourcePools",                  AIInterface::UpdateResourcePools);
        def("updateResearchQueue",                  AIInterface::UpdateResearchQueue);
        def("updateProductionQueue",                AIInterface::UpdateProductionQueue);

        def("issueFleetMoveOrder",                  AIInterface::IssueFleetMoveOrder, "Orders the fleet with indicated fleetID (int) to move to the system with the indicated destinationID (int). Returns 1 (int) on success or 0 (int) on failure due to not finding the indicated fleet or system.");
        def("issueRenameOrder",                     AIInterface::IssueRenameOrder, "Orders the renaming of the object with indicated objectID (int) to the new indicated name (string). Returns 1 (int) on success or 0 (int) on failure due to this AI player not being able to rename the indicated object (which this player must fully own, and which must be a fleet, ship or planet).");
        def("issueScrapOrder",                      AIIntScrap, "Orders the ship or building with the indicated objectID (int) to be scrapped. Returns 1 (int) on success or 0 (int) on failure due to not finding a ship or building with the indicated ID, or if the indicated ship or building is not owned by this AI client's empire.");
        def("issueNewFleetOrder",                   AIIntNewFleet, "Orders a new fleet to be created with the indicated name (string) and containing the indicated shipIDs (IntVec). The ships must be located in the same system and must all be owned by this player. Returns 1 (int) on success or 0 (int) on failure due to one of the noted conditions not being met.");
        def("issueFleetTransferOrder",              AIInterface::IssueFleetTransferOrder, "Orders the ship with ID shipID (int) to be transferred to the fleet with ID newFleetID. Returns 1 (int) on success, or 0 (int) on failure due to not finding the fleet or ship, or the client's empire not owning either, or the two not being in the same system (or either not being in a system) or the ship already being in the fleet.");
        def("issueColonizeOrder",                   AIInterface::IssueColonizeOrder, "Orders the ship with ID shipID (int) to colonize the planet with ID planetID (int). Returns 1 (int) on success or 0 (int) on failure due to not finding the indicated ship or planet, this client's player not owning the indicated ship, the planet already being colonized, or the planet and ship not being in the same system.");
        def("issueInvadeOrder",                     AIInterface::IssueInvadeOrder, "");
        def("issueBombardOrder",                    AIInterface::IssueBombardOrder, "");
        def("issueAggressionOrder",                 AIInterface::IssueAggressionOrder);
        def("issueGiveObjectToEmpireOrder",         AIInterface::IssueGiveObjectToEmpireOrder);
        def("issueChangeFocusOrder",                AIInterface::IssueChangeFocusOrder, "Orders the planet with ID planetID (int) to use focus setting focus (string). Returns 1 (int) on success or 0 (int) on failure if the planet can't be found or isn't owned by this player, or if the specified focus is not valid on the planet.");
        def("issueEnqueueTechOrder",                AIInterface::IssueEnqueueTechOrder, "Orders the tech with name techName (string) to be added to the tech queue at position (int) on the queue. Returns 1 (int) on success or 0 (int) on failure if the indicated tech can't be found. Will return 1 (int) but do nothing if the indicated tech can't be enqueued by this player's empire.");
        def("issueDequeueTechOrder",                AIInterface::IssueDequeueTechOrder, "Orders the tech with name techName (string) to be removed from the queue. Returns 1 (int) on success or 0 (int) on failure if the indicated tech can't be found. Will return 1 (int) but do nothing if the indicated tech isn't on this player's empire's tech queue.");
        def("issueEnqueueBuildingProductionOrder",  AIInterface::IssueEnqueueBuildingProductionOrder, "Orders the building with name (string) to be added to the production queue at the location of the planet with id locationID. Returns 1 (int) on success or 0 (int) on failure if there is no such building or it is not available to this player's empire, or if the building can't be produced at the specified location.");
        def("issueEnqueueShipProductionOrder",      AIInterface::IssueEnqueueShipProductionOrder, "Orders the ship design with ID designID (int) to be added to the production queue at the location of the planet with id locationID (int). Returns 1 (int) on success or 0 (int) on failure there is no such ship design or it not available to this player's empire, or if the design can't be produced at the specified location.");
        def("issueChangeProductionQuantityOrder",   AIInterface::IssueChangeProductionQuantityOrder);
        def("issueRequeueProductionOrder",          AIInterface::IssueRequeueProductionOrder, "Orders the item on the production queue at index oldQueueIndex (int) to be moved to index newQueueIndex (int). Returns 1 (int) on success or 0 (int) on failure if the old and new queue indices are equal, if either queue index is less than 0 or greater than the largest indexed item on the queue.");
        def("issueDequeueProductionOrder",          AIInterface::IssueDequeueProductionOrder, "Orders the item on the production queue at index queueIndex (int) to be removed form the production queue. Returns 1 (int) on success or 0 (int) on failure if the queue index is less than 0 or greater than the largest indexed item on the queue.");
        def("issueCreateShipDesignOrder",           IssueCreateShipDesignOrderWrapper, "Orders the creation of a new ship design with the name (string), description (string), hull (string), parts vector partsVec (StringVec), graphic (string) and model (string). model should be left as an empty string as of this writing. There is currently no easy way to find the id of the new design, though the client's empire should have the new design after this order is issued successfully. Returns 1 (int) on success or 0 (int) on failure if any of the name, description, hull or graphic are empty strings, if the design is invalid (due to not following number and type of slot requirements for the hull) or if creating the design fails for some reason.");

        def("sendChatMessage",          AIInterface::SendPlayerChatMessage, "Sends the indicated message (string) to the player with the indicated recipientID (int) or to all players if recipientID is -1.");
        def("sendDiplomaticMessage",    AIInterface::SendDiplomaticMessage);

        def("setSaveStateString",       SetStaticSaveStateString, "Sets the save state string (string). This is a persistant storage space for the AI script to retain state information when the game is saved and reloaded. Any AI state information to be saved should be stored in a single string (likely using Python's pickle module) and stored using this function when the prepareForSave() Python function is called.");
        def("getSaveStateString",       GetStaticSaveStateString,       return_value_policy<copy_const_reference>(), "Returns the previously-saved state string (string). Can be used to retrieve the last-set save state string at any time, although this string is also passed to the resumeLoadedGame(savedStateString) Python function when a game is loaded, so this function isn't necessary to use if resumeLoadedGame stores the passed string. ");

        def("doneTurn",                 AIInterface::DoneTurn, "Ends the AI player's turn, indicating to the server that all orders have been issued and turn processing may commence.");
        def("userString",               make_function(&UserString,          return_value_policy<copy_const_reference>()));
        def("userStringExists",         make_function(&UserStringExists,    return_value_policy<return_by_value>()));
        def("userStringList",           &GetUserStringList);

        def("getGalaxySetupData",       AIInterface::GetGalaxySetupData,    return_value_policy<copy_const_reference>());

        boost::python::scope().attr("INVALID_GAME_TURN") = INVALID_GAME_TURN;
    }
}
