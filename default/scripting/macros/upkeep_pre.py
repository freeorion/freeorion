# The formula defining the cost factor depending on the complexity
def DESIGN_SIMPLICITY_COMPLEXITY_FACTOR_FOR_ARG1_VREF_STATIC(arg: int) -> float:
    if arg > 4:
        return 1.0
    elif arg > 3:
        return 0.95
    elif arg > 2:
        return 0.9
    return 0.8
