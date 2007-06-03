import foaiint

def InitFreeOrionAI():
	foaiint.LogOutput("Initialized FreeOrion Python AI")
	foaiint.LogOutput(foaiint.PlayerName())

def GenerateOrders():
	#foaiint.SendChatMessage(0, "Sending chat message from within Python!")
	
	empire = foaiint.GetEmpire()
	universe = foaiint.GetUniverse()
	object_ids = universe.ObjectIDs()
	
	for obj_id in object_ids:
	    ship = universe.GetShip(obj_id)
	    
	    if ship == None: continue
	    
	    foaiint.LogOutput("ID " + str(obj_id) + " is a ship!")


	#homeworldID = empire.HomeworldID()
	#foaiint.SendChatMessage(0, "My homeworld id: " + str(empire.HomeworldID()))	
	
	foaiint.DoneTurn()
	foaiint.LogOutput("Generated Orders")
