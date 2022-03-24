from freeorion_tools.lazy_initializer import InitializerLock

# put in an extra file only to avoid circular includes
survey_universe_lock = InitializerLock("survery_universe")
# some values are used to calculate others, so we need two locks
survey_universe_lock2 = InitializerLock("survery_universe2")
