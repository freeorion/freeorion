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
        for (std::list< std::string >::iterator it = retval.begin(); it != retval.end(); it++)
            ret_list.append(*it);
        return ret_list;
    }

    boost::python::str GetUserDirWrapper()
    { return boost::python::str(PathString(GetUserDir())); }

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
        def("playerName",               AIIntPlayerNameVoid,            return_value_policy<copy_const_reference>());
        def("playerName",               AIIntPlayerNameInt,             return_value_policy<copy_const_reference>());

        def("playerID",                 AIInterface::PlayerID);
        def("empirePlayerID",           AIInterface::EmpirePlayerID);
        def("allPlayerIDs",             AIInterface::AllPlayerIDs,      return_value_policy<return_by_value>());

        def("playerIsAI",               AIInterface::PlayerIsAI);
        def("playerIsHost",             AIInterface::PlayerIsHost);

        def("empireID",                 AIInterface::EmpireID);
        def("playerEmpireID",           AIInterface::PlayerEmpireID);
        def("allEmpireIDs",             AIInterface::AllEmpireIDs,      return_value_policy<return_by_value>());

        def("getEmpire",                AIIntGetEmpireVoid,             return_value_policy<reference_existing_object>());
        def("getEmpire",                AIIntGetEmpireInt,              return_value_policy<reference_existing_object>());

        def("getUniverse",              AIInterface::GetUniverse,       return_value_policy<reference_existing_object>());

        def("currentTurn",              AIInterface::CurrentTurn);

        def("getAIConfigStr",           AIInterface::GetAIConfigStr,    return_value_policy<return_by_value>());
        def("getAIDir",                 AIInterface::GetAIDir,          return_value_policy<return_by_value>());
        def("getUserDir",               GetUserDirWrapper,              /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific data (config files, saves, etc.).");

        def("initMeterEstimatesDiscrepancies",      AIInterface::InitMeterEstimatesAndDiscrepancies);
        def("updateMeterEstimates",                 AIInterface::UpdateMeterEstimates);
        def("updateResourcePools",                  AIInterface::UpdateResourcePools);
        def("updateResearchQueue",                  AIInterface::UpdateResearchQueue);
        def("updateProductionQueue",                AIInterface::UpdateProductionQueue);

        def("issueFleetMoveOrder",                  AIInterface::IssueFleetMoveOrder);
        def("issueRenameOrder",                     AIInterface::IssueRenameOrder);
        def("issueScrapOrder",                      AIIntScrap);
        def("issueNewFleetOrder",                   AIIntNewFleet);
        def("issueFleetTransferOrder",              AIInterface::IssueFleetTransferOrder);
        def("issueColonizeOrder",                   AIInterface::IssueColonizeOrder);
        def("issueInvadeOrder",                     AIInterface::IssueInvadeOrder);
        def("issueBombardOrder",                    AIInterface::IssueBombardOrder);
        def("issueAggressionOrder",                 AIInterface::IssueAggressionOrder);
        def("issueGiveObjectToEmpireOrder",         AIInterface::IssueGiveObjectToEmpireOrder);
        def("issueChangeFocusOrder",                AIInterface::IssueChangeFocusOrder);
        def("issueEnqueueTechOrder",                AIInterface::IssueEnqueueTechOrder);
        def("issueDequeueTechOrder",                AIInterface::IssueDequeueTechOrder);
        def("issueEnqueueBuildingProductionOrder",  AIInterface::IssueEnqueueBuildingProductionOrder);
        def("issueEnqueueShipProductionOrder",      AIInterface::IssueEnqueueShipProductionOrder);
        def("issueChangeProductionQuantityOrder",   AIInterface::IssueChangeProductionQuantityOrder);
        def("issueRequeueProductionOrder",          AIInterface::IssueRequeueProductionOrder);
        def("issueDequeueProductionOrder",          AIInterface::IssueDequeueProductionOrder);
        def("issueCreateShipDesignOrder",           IssueCreateShipDesignOrderWrapper);

        def("sendChatMessage",          AIInterface::SendPlayerChatMessage);
        def("sendDiplomaticMessage",    AIInterface::SendDiplomaticMessage);

        def("setSaveStateString",       SetStaticSaveStateString);
        def("getSaveStateString",       GetStaticSaveStateString,       return_value_policy<copy_const_reference>());

        def("doneTurn",                 AIInterface::DoneTurn);
        def("userString",               make_function(&UserString,          return_value_policy<copy_const_reference>()));
        def("userStringExists",         make_function(&UserStringExists,    return_value_policy<return_by_value>()));
        def("userStringList",           &GetUserStringList);

        def("getGalaxySetupData",       AIInterface::GetGalaxySetupData,    return_value_policy<copy_const_reference>());

        boost::python::scope().attr("INVALID_GAME_TURN") = INVALID_GAME_TURN;
    }
}
