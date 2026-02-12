from focs._effects import (
    Conditional,
    CurrentTurn,
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    GenerateSitRepMessage,
    EmpireMeterValue,
    IsSource,
    SetEmpireMeter,
    Source,
    Value,
)

racial_purity_once_per_turn_effectgroups = [
    EffectsGroup(
        scope=IsSource,
        effects=Conditional(
            condition=(EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_RACIAL_PURITY")),
            effects=[
                #SetEmpireMeter(empire=Source.Owner, meter="PLC_RACIAL_PURITY_TURN_ACTIVE", value=CurrentTurn),
                SetEmpireMeter(empire=Source.Owner, meter="PLC_RACIAL_PURITY_TURN_ACTIVE", value=0.0+CurrentTurn),
            ],
            else_=[
                SetEmpireMeter(empire=Source.Owner, meter="SOCIAL_CATEGORY_NUM_POLICY_SLOTS", value=Value+1),
                GenerateSitRepMessage(
                    message="Test ; last rac pur turn: %lastturn%",
                    label="SITREP_WELCOME_LABEL",
                    NoStringtableLookup=True,
                    icon="icons/tech/categories/spy.png",
                    parameters={"lastturn": EmpireMeterValue(empire=Source.Owner, meter="PLC_RACIAL_PURITY_TURN_ACTIVE")},
                    empire=Source.Owner,
                ),
            ],
        )
    ),
]
