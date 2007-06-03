import foaiint

def InitFreeOrionAI():
	foaiint.LogOutput("Initialized FreeOrion Python AI")
	foaiint.LogOutput(foaiint.PlayerName())

def GenerateOrders():
	#foaiint.SendChatMessage(0, "Sending chat message from within Python!")
	
	empire = foaiint.GetEmpire()
	empire_id = foaiint.EmpireID()
	universe = foaiint.GetUniverse()
	object_ids_list = universe.ObjectIDs()
	
	[fleet_ids_list, system_ids_list] = GetFleetAndSystemIDs(object_ids_list)
	
    
	#foaiint.SendChatMessage(0, "My homeworld id: " + str(empire.HomeworldID()))	
	
	foaiint.DoneTurn()
	foaiint.LogOutput("Generated Orders")

	
def GetFleetAndSystemIDs(object_ids_list):
	empire = foaiint.GetEmpire()
	empire_id = foaiint.EmpireID()
	universe = foaiint.GetUniverse()

	fleet_ids_list = []
	system_ids_list = []

	for obj_id in object_ids_list:
	    fleet = universe.GetFleet(obj_id)
	    if (fleet != None):
			if (not fleet.OwnedWhollyBy(empire_id)): continue	    
			fleet_ids_list = fleet_ids_list + [obj_id]

		system = universe.GetSystem(obj_id)
		if (system != None):
			if (not universe.SystemReachable(obj_id, empire_id)): continue
			if (empire.HasExploredSystem(system_id)): continue
			system_ids_list = system_ids_list + [obj_id]

	return [fleet_ids_list, system_ids_list]
