from common.configure_logging import redirect_logging_to_freeorion_logger, convenience_function_references_for_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()
(debug, info, warn, error, fatal) = convenience_function_references_for_logger()

import sys

class AuthProvider:
    def __init__(self):
        self.logins = { 'test' : 'test' }
        print >> sys.stdout, "Auth initialized"
 
    def is_require_auth(self, player_name):
        return player_name in self.logins

    def is_success_auth(self, player_name, auth):
        return self.logins.get(player_name) == auth

