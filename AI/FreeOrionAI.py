import foaiint

def InitFreeOrionAI():
	foaiint.LogOutput("Initialized FreeOrion Python AI")

def GenerateOrders():
	foaiint.DoneTurn()
	foaiint.SendChatMessage(0, "Sending chat message from within Python!")
	foaiint.LogOutput("Generated Orders")