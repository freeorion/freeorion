from common.configure_logging import redirect_logging_to_freeorion_logger, convenience_function_references_for_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()
(debug, info, warn, error, fatal) = convenience_function_references_for_logger()

import sys

import freeorion as fo

class AuthProvider:
    def __init__(self):
        self.logins = {}
        try:
            with open(fo.get_user_config_dir() + "/auth.txt") as f:
                for line in f:
                    l = line.rsplit(':', 1)
                    self.logins[l[0]] = l[1]
        except:
            exctype, value = sys.exc_info()[:2]
            warn("Cann't read auth file %s: %s %s" % (fo.get_user_config_dir() + "/auth.txt", exctype, value))
        info("Auth initialized")
 
    def is_require_auth(self, player_name):
        return player_name in self.logins

    def is_success_auth(self, player_name, auth):
        return self.logins.get(player_name) == auth

