import freeOrionAIInterface as fo

def generateResearchOrders():
    "generate research orders"
    
    print "Research:"
    
    empire = fo.getEmpire()
    availableRP = empire.resourceProduction(fo.resourceType.research)
    print "total Research Points: " + str(availableRP)
    print "spent Research Points: " + str(totalSpentRP())
    
    # get all researchable techs not already queued for research
    possibleResearchProjects = getPossibleProjects()
    empire = fo.getEmpire()
    researchQueue = empire.researchQueue
    for element in researchQueue:
        possibleResearchProjects.remove(element.tech.name)
        
    print "projects already in research queue: "
    for element in researchQueue:
        print "    " + element.tech.name 

    print "possible new research projects:"
    cheapestProject = None
    INFINITY = 9999
    cheapestCost = INFINITY
    for project in possibleResearchProjects:
        tech = fo.getTech(project)
        print "    " + tech.name + " ["+ tech.category + "] cost:"+ str(tech.researchCost)
        
        if (cheapestCost > tech.researchCost):
            cheapestCost = tech.researchCost
            cheapestProject = project
            
    print ""
    if INFINITY > cheapestCost and totalSpentRP() < availableRP:
        tech = fo.getTech(cheapestProject)
        print "adding new research project: " + tech.name + " to queue"
        fo.issueEnqueueTechOrder(cheapestProject,-1)
    
    print ""
    
def getPossibleProjects():
    "get possible projects"
    
    possibleProjects = []
    technames = fo.techs()
    empire = fo.getEmpire()
    for techname in technames:
        if empire.getTechStatus(techname) == fo.techStatus.researchable:
            possibleProjects.append(techname)
            
    return possibleProjects

def totalSpentRP():
    "calculate RPs spent this turn so far"
    
    queue=fo.getEmpire().researchQueue
    total = 0
    for element in queue:
        total = total + element.allocation
    return total
