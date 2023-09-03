from focs._effects import (
    Capital,
    Contains,
    EmpireHasAdoptedPolicy,
    HasSpecial,
    HasSpecies,
    HasTag,
    Homeworld,
    IsBuilding,
    LocalCandidate,
    Mode,
    OwnedBy,
    OwnerHasTech,
    Planet,
    Source,
    Statistic,
)
from focs._species import *

HAS_INDUSTRY_FOCUS = FocusType(
    name="FOCUS_INDUSTRY", description="FOCUS_INDUSTRY_DESC", location=Planet(), graphic="icons/focus/industry.png"
)

HAS_RESEARCH_FOCUS = FocusType(
    name="FOCUS_RESEARCH",
    description="FOCUS_RESEARCH_DESC",
    location=Planet()
    & (
        ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_RACIAL_PURITY")
        | HasSpecies(name=["SP_EXOBOT"])
        | HasSpecies(
            name=[Statistic(str, Mode, value=LocalCandidate.Species, condition=Capital & OwnedBy(empire=Source.Owner))]
        )
    ),
    graphic="icons/focus/research.png",
)

HAS_INFLUENCE_FOCUS = FocusType(
    name="FOCUS_INFLUENCE",
    description="FOCUS_INFLUENCE_DESC",
    location=Planet()
    & (
        ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_RACIAL_PURITY")
        | HasSpecies(name=["SP_EXOBOT"])
        | HasSpecies(
            name=[Statistic(str, Mode, value=LocalCandidate.Species, condition=Capital & OwnedBy(empire=Source.Owner))]
        )
    ),
    graphic="icons/focus/influence.png",
)

HAS_GROWTH_FOCUS = FocusType(
    name="FOCUS_GROWTH",
    description="FOCUS_GROWTH_DESC",
    location=(Homeworld(name=[Source.Species]) & ~HasTag(name="SELF_SUSTAINING"))
    | HasSpecial(name="POSITRONIUM_SPECIAL")
    | HasSpecial(name="SUPERCONDUCTOR_SPECIAL")
    | HasSpecial(name="MONOPOLE_SPECIAL")
    | HasSpecial(name="SPICE_SPECIAL")
    | HasSpecial(name="FRUIT_SPECIAL")
    | HasSpecial(name="PROBIOTIC_SPECIAL")
    | HasSpecial(name="ELERIUM_SPECIAL")
    | HasSpecial(name="CRYSTALS_SPECIAL")
    | HasSpecial(name="MINERALS_SPECIAL"),
    graphic="icons/focus/growth.png",
)

HAS_ADVANCED_FOCI = [
    FocusType(
        name="FOCUS_PROTECTION",
        description="FOCUS_PROTECTION_DESC",
        # location = OwnerHasTech name = "DEF_ROOT_DEFENSE"
        location=Planet(),  # empires have self-defense anyway, and this way some natives can now defend themselves better
        graphic="icons/focus/protection.png",
    ),
    FocusType(
        name="FOCUS_LOGISTICS",
        description="FOCUS_LOGISTICS_DESC",
        location=OwnerHasTech(name="SHP_INTSTEL_LOG"),
        graphic="icons/focus/supply.png",
    ),
    FocusType(
        name="FOCUS_STOCKPILE",
        description="FOCUS_STOCKPILE_DESC",
        location=OwnerHasTech(name="PRO_GENERIC_SUPPLIES"),
        graphic="icons/focus/stockpile.png",
    ),
    FocusType(
        name="FOCUS_STEALTH",
        description="FOCUS_STEALTH_DESC",
        location=Contains(IsBuilding(name=["BLD_PLANET_CLOAK"]))
        | (Contains(IsBuilding(name=["BLD_TRANSFORMER"])) & OwnerHasTech(name="DEF_PLANET_CLOAK")),
        graphic="icons/focus/stealth.png",
    ),
    FocusType(
        name="FOCUS_BIOTERROR",
        description="FOCUS_BIOTERROR_DESC",
        location=Contains(IsBuilding(name=["BLD_BIOTERROR_PROJECTOR"]))
        | (Contains(IsBuilding(name=["BLD_TRANSFORMER"])) & OwnerHasTech(name="GRO_BIOTERROR")),
        graphic="icons/focus/bioterror.png",
    ),
    FocusType(
        name="FOCUS_STARGATE_SEND",
        description="FOCUS_STARGATE_SEND_DESC",
        location=Contains(IsBuilding(name=["BLD_STARGATE"]))
        | (Contains(IsBuilding(name=["BLD_TRANSFORMER"])) & OwnerHasTech(name="CON_STARGATE")),
        graphic="icons/focus/stargate_send.png",
    ),
    FocusType(
        name="FOCUS_STARGATE_RECEIVE",
        description="FOCUS_STARGATE_RECEIVE_DESC",
        location=Contains(IsBuilding(name=["BLD_STARGATE"]))
        | (Contains(IsBuilding(name=["BLD_TRANSFORMER"])) & OwnerHasTech(name="CON_STARGATE")),
        graphic="icons/focus/stargate_receive.png",
    ),
    FocusType(
        name="FOCUS_PLANET_DRIVE",
        description="FOCUS_PLANET_DRIVE_DESC",
        location=Contains(IsBuilding(name=["BLD_PLANET_DRIVE"]))
        | (Contains(IsBuilding(name=["BLD_TRANSFORMER"])) & OwnerHasTech(name="CON_PLANET_DRIVE")),
        graphic="icons/building/planetary_stardrive.png",
    ),
    FocusType(
        name="FOCUS_DISTORTION",
        description="FOCUS_DISTORTION_DESC",
        location=Contains(IsBuilding(name=["BLD_SPATIAL_DISTORT_GEN"]))
        | (Contains(IsBuilding(name=["BLD_TRANSFORMER"])) & OwnerHasTech(name="LRN_SPATIAL_DISTORT_GEN")),
        graphic="icons/focus/distortion.png",
    ),
    FocusType(
        name="FOCUS_DOMINATION",
        description="FOCUS_DOMINATION_DESC",
        location=HasTag(name="TELEPATHIC") & OwnerHasTech(name="LRN_PSY_DOM"),
        graphic="icons/focus/psi_domination.png",
    ),
]
