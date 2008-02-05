import FreeOrionAIInterface as fo


# called when Python AI starts
def InitFreeOrionAI():
    fo.LogOutput("Initialized FreeOrion Python AI")
    fo.LogOutput(fo.PlayerName())


# called once per turn
def GenerateOrders():
    fo.LogOutput("Generating Orders")

    empire = fo.GetEmpire()
    empire_id = fo.EmpireID()
    universe = fo.GetUniverse()

    # get stationary fleets
    fleet_ids_list = GetEmpireStationaryFleetIDs(empire_id)
    fo.LogOutput("fleet_ids_list: " + str(fleet_ids_list))


    for fleet_id in fleet_ids_list:
        fleet = universe.GetFleet(fleet_id)
        if (fleet == None): continue

        fo.LogOutput("Fleet: " + str(fleet_id));

        start_system_id = fleet.systemID
        if (start_system_id == fleet.INVALID_OBJECT_ID): continue

        fo.LogOutput("in system: " + str(start_system_id));

        system_ids_list = GetExplorableSystemIDs(start_system_id, empire_id)

        fo.LogOutput("can explore: " + str(system_ids_list));

        if (len(system_ids_list) > 0):
            destination_id = system_ids_list[0]
            fo.IssueFleetMoveOrder(fleet_id, destination_id)


    fo.DoneTurn()


# returns list of systems ids known of by but not explored by empire_id,
# that a ship located in start_system_id could reach via starlanes
def GetExplorableSystemIDs(start_system_id, empire_id):
    fo.LogOutput("GetExplorableSystemIDs")
    universe = fo.GetUniverse()
    object_ids_vec = universe.allObjectIDs
    empire = fo.GetEmpire(empire_id)

    system_ids_list = []


    for obj_id in object_ids_vec:
        fo.LogOutput("GetExplorableSystemIDs object id: " + str(obj_id))

        system = universe.GetSystem(obj_id)
        if (system == None): continue

        fo.LogOutput("...is a system")

        if (empire.HasExploredSystem(obj_id)): continue

        fo.LogOutput("...not explored")

        if (not universe.SystemsConnected(obj_id, start_system_id, empire_id)): continue

        fo.LogOutput("...is connected to start system " + str(start_system_id))

        system_ids_list = system_ids_list + [obj_id]

    return system_ids_list


# returns list of staitionary fleet ids owned by empire_id
def GetEmpireStationaryFleetIDs(empire_id):
    fo.LogOutput("GetEmpireFleetIDs")
    universe = fo.GetUniverse()
    object_ids_vec = universe.allObjectIDs

    fleet_ids_list = []

    for obj_id in object_ids_vec:
        fleet = universe.GetFleet(obj_id)
        if (fleet == None): continue

        if (not fleet.WhollyOwnedBy(empire_id)): continue

        if (fleet.nextSystemID != fleet.INVALID_OBJECT_ID): continue

        fleet_ids_list = fleet_ids_list + [obj_id]

    return fleet_ids_list

