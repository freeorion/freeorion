# Mock module for tests


class aggression(object):
    """Aggression enumeration."""
    beginner = 0
    turtle = 1
    cautious = 2
    typical = 3
    aggressive = 4
    maniacal = 5


class planetSize(object):
    """PlanetSize enumeration."""
    tiny = 1
    small = 2
    medium = 3
    large = 4
    huge = 5
    asteroids = 6
    gasGiant = 7
    noWorld = 0
    unknown = -1


class starType(object):
    """StarType enumeration."""
    blue = 0
    white = 1
    yellow = 2
    orange = 3
    red = 4
    neutron = 5
    blackHole = 6
    noStar = 7
    unknown = -1


class visibility(object):
    """Visibility enumeration."""
    invalid = -1
    none = 0
    basic = 1
    partial = 2
    full = 3


class planetType(object):
    """PlanetType enumeration."""
    # this is the listing order in EnumWrapper.cpp, so keeping it the same here
    swamp = 0
    radiated = 3
    toxic = 1
    inferno = 2
    barren = 4
    tundra = 5
    desert = 6
    terran = 7
    ocean = 8
    asteroids = 9
    gasGiant = 10
    unknown = -1


class planetEnvironment(object):
    """PlanetEnvironment enumeration."""
    uninhabitable = 0
    hostile = 1
    poor = 2
    adequate = 3
    good = 4


class techStatus(object):
    """TechStatus enumeration."""
    unresearchable = 0
    partiallyUnlocked = 1
    researchable = 2
    complete = 3


class buildType(object):
    """BuildType enumeration."""
    building = 0
    ship = 1
    stockpile = 2


class resourceType(object):
    """ResourceType enumeration."""
    industry = 0
    trade = 1
    research = 2
    stockpile = 3


class meterType(object):
    """MeterType enumeration."""
    targetPopulation = 0
    targetIndustry = 1
    targetResearch = 2
    targetTrade = 3
    targetConstruction = 4
    targetHappiness = 5
    maxDamage = 6
    maxCapacity = 6
    maxSecondaryStat = 7
    maxFuel = 8
    maxShield = 9
    maxStructure = 10
    maxDefense = 11
    maxSupply = 12
    maxStockpile = 13
    maxTroops = 14
    population = 15
    industry = 16
    research = 17
    trade = 18
    construction = 19
    happiness = 20
    damage = 21
    capacity = 21
    secondaryStat = 22
    fuel = 23
    shield = 24
    structure = 25
    defense = 26
    supply = 27
    stockpile = 28
    troops = 29
    rebels = 30
    size = 31
    stealth = 32
    detection = 33
    speed = 34


class diplomaticStatus(object):
    """DiplomaticStatus enumeration."""
    war = 0
    peace = 1
    allied = 2


class diplomaticMessageType(object):
    """DiplomaticMessageType enumeration."""
    noMessage = -1
    warDeclaration = 0
    peaceProposal = 1
    acceptPeaceProposal = 2
    alliesProposal = 3
    acceptAlliesProposal = 4
    endAllies = 5
    cancelProposal = 6
    rejectProposal = 7


class captureResult(object):
    """CaptureResult enumeration."""
    capture = 0
    destroy = 1
    retain = 2


class effectsCauseType(object):
    """EffectsCauseType enumeration."""
    invalid = -1
    unknown = 0
    inherent = 1
    tech = 2
    building = 3
    field = 4
    special = 5
    species = 6
    shipPart = 7
    shipHull = 8


class shipSlotType(object):
    """ShipSlotType enumeration."""
    external = 0
    internal = 1
    core = 2


class shipPartClass(object):
    """ShipPartClass enumeration."""
    shortRange = 0
    fighterBay = 1
    fighterHangar = 2
    shields = 3
    armour = 4
    troops = 5
    detection = 6
    stealth = 7
    fuel = 8
    colony = 9
    speed = 10
    general = 11
    bombard = 12
    industry = 13
    research = 14
    trade = 15
    productionLocation = 16


class unlockableItemType(object):
    """UnlockableItemType enumeration."""
    invalid = -1
    building = 0
    shipPart = 1
    shipHull = 2
    shipDesign = 3
    tech = 4


class galaxySetupOption(object):
    """GalaxySetupOption enumeration."""
    invalid = -1
    none = 0
    low = 1
    medium = 2
    high = 3
    random = 4


class galaxyShape(object):
    """GalaxyShape enumeration."""
    invalid = -1
    spiral2 = 0
    spiral3 = 1
    spiral4 = 2
    cluster = 3
    elliptical = 4
    disc = 5
    box = 6
    irregular = 7
    ring = 8
    random = 9


class ruleType(object):
    """RuleType enumeration."""
    invalid = -1
    toggle = 0
    int = 1
    double = 2
    string = 3


class roleType(object):
    """RoleType enumeration."""
    host = 0
    clientTypeModerator = 1
    clientTypePlayer = 2
    clientTypeObserver = 3
    galaxySetup = 4


def userString(x):
    """userString mock"""
    return "UserString %s" % x


def userStringList(x):
    """userStringList mock"""
    return "UserStringList %s" % x
