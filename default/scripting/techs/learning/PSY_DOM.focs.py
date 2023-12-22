from focs._effects import (
    ContainedBy,
    Contains,
    EffectsGroup,
    Focus,
    GenerateSitRepMessage,
    HasTag,
    Monster,
    OwnedBy,
    OwnerHasTech,
    Planet,
    Random,
    SetOwner,
    Ship,
    Source,
    System,
    Target,
    VisibleToEmpire,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_PSY_DOM",
    description="LRN_PSY_DOM_DESC",
    short_description="MIND_CONTROL_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=1050 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_UNIF_CONC", "LRN_TIME_MECH"],
    effectsgroups=[
        EffectsGroup(
            scope=Ship
            & ~OwnedBy(empire=Source.Owner)
            & VisibleToEmpire(empire=Source.Owner)
            & Random(probability=0.1)
            & ~Monster
            &
            # @content_tag{TELEPATHIC} For ships with this tag, prevents chance of ownership loss to suitable planet
            ~HasTag(name="TELEPATHIC")
            & ~OwnerHasTech(name="LRN_PSY_DOM")
            & ContainedBy(
                System
                & Contains(
                    Planet()
                    & OwnedBy(empire=Source.Owner)
                    &
                    # @content_tag{TELEPATHIC} On owned planet with focus FOCUS_DOMINATION and this tag, chance of taking ownership of suitable ships
                    HasTag(name="TELEPATHIC")
                    & Focus(type=["FOCUS_DOMINATION"])
                )
            ),
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_PSY_DOM",
                    label="EFFECT_PSY_DOM_LABEL",
                    icon="icons/sitrep/ship_produced.png",
                    parameters={"empire": Target.Owner, "ship": Target.ID},
                    empire=Target.Owner,
                ),
                GenerateSitRepMessage(
                    message="EFFECT_PSY_DOM",
                    label="EFFECT_PSY_DOM_LABEL",
                    icon="icons/sitrep/ship_produced.png",
                    parameters={"empire": Target.Owner, "ship": Target.ID},
                    empire=Source.Owner,
                ),
                SetOwner(empire=Source.Owner),
            ],
        )
    ],
    graphic="icons/tech/psychogenic_domination.png",
)
