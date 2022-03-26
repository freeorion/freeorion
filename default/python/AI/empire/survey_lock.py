from freeorion_tools.lazy_initializer import InitializerLock

# put in an extra file only to avoid circular includes
survey_universe_lock = InitializerLock("survery_universe")
