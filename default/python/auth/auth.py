from common.configure_logging import redirect_logging_to_freeorion_logger, convenience_function_references_for_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()
(debug, info, warn, error, fatal) = convenience_function_references_for_logger()

import sys

import freeorion as fo


class AuthProvider:
    def __init__(self):
        self.logins = {}
        rolesSymbols = {'h' : fo.roleType.host, 'm' : fo.roleType.clientTypeModerator, 'p' : fo.roleType.clientTypePlayer, 'o' : fo.roleType.clientTypeObserver, 'g' : fo.roleType.galaxySetup }
        try:
            with open(fo.get_user_config_dir() + "/auth.txt") as f:
                for line in f:
                    l = line.rsplit(':', 2)
                    roles = []
                    for c in l[1]:
                        r = rolesSymbols.get(c)
                        if r is None:
                            warn("unknown role symbol '%c' for login %s" % (c, l[0]))
                        else:
                            roles.append(r)
                    self.logins[l[0]] = (l[2].strip(), roles)
        except IOError:
            exctype, value = sys.exc_info()[:2]
            warn("Cann't read auth file %s: %s %s" % (fo.get_user_config_dir() + "/auth.txt", exctype, value))
        info("Auth initialized")

    def is_require_auth_or_return_roles(self, player_name):
        """Returns True if player should be authenticated or list of roles for anonymous players"""
        known_login = player_name in self.logins
        if not known_login:
            # default list of roles
            return [ fo.roleType.clientTypeModerator, fo.roleType.clientTypePlayer, fo.roleType.clientTypeObserver, fo.roleType.galaxySetup ]
        return True

    def is_success_auth_and_return_roles(self, player_name, auth):
        """Return False if passowrd doesn't match or list of roles for authenticated player"""
        auth_data = self.logins.get(player_name)
        if auth_data[0] == auth:
            return auth_data[1]
        else:
            return False
        
