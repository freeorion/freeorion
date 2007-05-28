import foaiint

def InitFreeOrionAI():
	foaiint.LogOutput("Initialized FreeOrion Python AI")
	foaiint.LogOutput(foaiint.PlayerName())

def GenerateOrders():
	foaiint.SendChatMessage(0, "Sending chat message from within Python!")
	empire = foaiint.GetEmpire()
	homeworldID = empire.HomeworldID()
	foaiint.SendChatMessage(0, "My homeworld id: " + str(empire.HomeworldID()))
	foaiint.DoneTurn()
	foaiint.LogOutput("Generated Orders")
