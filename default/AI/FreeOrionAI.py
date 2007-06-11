import foaiint

# globals
systems_to_explore = []
systems_being_explored = []


# called when Python AI starts
def InitFreeOrionAI():
    foaiint.LogOutput("Initialized FreeOrion Python AI")
    foaiint.LogOutput(foaiint.PlayerName())


# called once per turn
def GenerateOrders():
    foaiint.LogOutput("Generating Orders")
    
    empire = foaiint.GetEmpire()
    empire_id = foaiint.EmpireID()
    universe = foaiint.GetUniverse()
    
    # get stationary fleets
    fleet_ids_list = GetEmpireStationaryFleetIDs(empire_id)
    foaiint.LogOutput("fleet_ids_list: " + str(fleet_ids_list))


    for fleet_id in fleet_ids_list:
        fleet = universe.GetFleet(fleet_id)
        if (fleet == None): continue
        
        start_system_id = fleet.SystemID()
        if (start_system_id == fleet.INVALID_OBJECT_ID): continue
        
        system_ids_list = GetExplorableSystemIDs(start_system_id, empire_id)

        
    foaiint.LogOutput("system_ids_list: " + str(system_ids_list))
        
    foaiint.DoneTurn()


# returns list of systems ids known of by but not explored by empire_id,
# that a ship located in start_system_id could reach via starlanes
def GetExplorableSystemIDs(start_system_id, empire_id):
    foaiint.LogOutput("GetExplorableSystemIDs")
    universe = foaiint.GetUniverse()
    object_ids_vec = universe.ObjectIDs()
    empire = foaiint.GetEmpire(empire_id)
    
    system_ids_list = []


    for obj_id in object_ids_vec:
        foaiint.LogOutput("GetExplorableSystemIDs objedt id: " + str(obj_id))
        
        system = universe.GetSystem(obj_id)
        if (system == None): continue

        foaiint.LogOutput("...is a system")

        if (empire.HasExploredSystem(obj_id)): continue

        foaiint.LogOutput("...not explored")

        if (not universe.SystemsConnected(obj_id, start_system_id, empire_id)): continue

        foaiint.LogOutput("...is connected to start system " + str(start_system_id))
        
        system_ids_list = system_ids_list + [obj_id]
    
    return system_ids_list


# returns list of staitionary fleet ids owned by empire_id
def GetEmpireStationaryFleetIDs(empire_id):
    foaiint.LogOutput("GetEmpireFleetIDs")
    universe = foaiint.GetUniverse()
    object_ids_vec = universe.ObjectIDs()
    
    fleet_ids_list = []

    for obj_id in object_ids_vec:
        fleet = universe.GetFleet(obj_id)
        if (fleet == None): continue

        if (not fleet.WhollyOwnedBy(empire_id)): continue

        if (fleet.NextSystemID() != fleet.INVALID_OBJECT_ID): continue

        fleet_ids_list = fleet_ids_list + [obj_id]

    return fleet_ids_list


#def GetExplorableSystemID(start_system_id, object_ids_list):
#    foaiint.LogOutput("GetExplorableSystemID")
#    universe = foaiint.GetUniverse()
#    empire_id = foaiint.EmpireID()
#    
#    system_ids_list = []
#    
#    foaiint.LogOutput("A")
#    for obj_id in object_ids_list:
#        foaiint.LogOutput("B")
#        
 #       system = universe.GetSystem(obj_id)        
 #       if (system == None): continue
 #       
 #       foaiint.LogOutput("C")
#        
 #       if (not universe.SystemsConnected(start_system_id, obj_id, empire_id)): continue
 #       
 #       foaiint.LogOutput("D")
 #       
 #       system_ids_list = system_ids_list + [obj_id]
 #       
 #       foaiint.LogOutput("E")
 #   
 #   foaiint.LogOutput("F")
 #   return system_ids_list
