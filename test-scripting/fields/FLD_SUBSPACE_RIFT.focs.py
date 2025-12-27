from focs._fields import FieldType

try:
    from focs._fields import *
except ModuleNotFoundError:
    pass

FieldType(
    name="FLD_SUBSPACE_RIFT",
    description="FLD_SUBSPACE_RIFT_DESC",
    stealth=0,
    effectsgroups=[
        EffectsGroup(  # pull in objects
            scope=System & ~Contains(IsSource) & WithinDistance(distance=Source.Size, condition=IsSource),
            effects=MoveTowards(speed=5, target=IsSource),
        ),
        EffectsGroup(  # destroy close objects
            scope=~IsSource & ~Contains(IsSource) & WithinDistance(distance=10, condition=IsSource), effects=Destroy
        ),
        EffectsGroup(  # shrink at same speed objects are pulled
            scope=IsSource, effects=SetSize(value=Target.Size - 5)
        ),
        EffectsGroup(  # collapse upon self when small enough
            scope=IsSource, activation=Size(high=5), effects=Destroy
        ),
    ],
    graphic="nebulae/nebula9.png",
)
