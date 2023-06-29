from common.priorities import DEFAULT_PRIORITY

ULTIMATE_DETECTION = EffectsGroup(
    description="ULTIMATE_DETECTION_DESC", scope=IsSource, effects=SetDetection(value=Value + 100)
)


def NATIVE_PLANETARY_DETECTION(detection):
    return EffectsGroup(
        scope=IsSource,
        activation=Planet() & Unowned,
        accountinglabel="NATIVE_PLANETARY_DETECTION_LABEL",
        priority=DEFAULT_PRIORITY,
        effects=SetDetection(value=Value + detection),
    )
