import freeOrionAIInterface as fo

def generateResearchOrders():
    "generate research orders"

    print "Research:"

    empire = fo.getEmpire()
    totalRP = empire.resourceProduction(fo.resourceType.research)


    # get all researchable techs not already queued for research
    possibleResearchProjects = getPossibleProjects()
    empire = fo.getEmpire()
    researchQueue = empire.researchQueue
    for element in researchQueue:
        possibleResearchProjects.remove(element.tech.name)

    print "projects already in research queue: "
    for element in researchQueue:
        print "    " + element.tech.name 


    # store projects mapped to their costs, so they can be sorted by that cost
    print "possible projects"
    projectsDict = dict()
    for name in possibleResearchProjects:
        projectsDict[name] = fo.getTech(name).researchCost


    # iterate through techs in order of cost
    print "enqueuing techs.  already spent RP: " + str(spentRP()) + "  total RP: " + str(totalRP)
    for name, cost in sorted(projectsDict.items(), key=lambda(k,v):(v,k)):
        # abort if no RP left
        if spentRP() >= totalRP:
            break

        # add tech to queue
        fo.issueEnqueueTechOrder(name, -1)
        print "    enqueued tech " + name + "  :  cost: " + str(cost) + "RP"

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

def spentRP():
    "calculate RPs spent this turn so far"
    
    queue = fo.getEmpire().researchQueue
    return queue.totalSpent

