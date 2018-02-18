"""
Config for tests inside this directory.
https://docs.pytest.org/en/2.7.3/plugins.html?highlight=re#working-with-plugins-and-conftest-files
"""

import os
import sys

this_dir = os.path.dirname(__file__)

sys.path.append(this_dir)
sys.path.append(os.path.join(this_dir, '..', '..', 'python', 'AI/'))
sys.path.append(os.path.join(this_dir, '..', '..', 'python'))

# Since the 'true' freeOrionAIInterface is not available during testing, this import loads the 'mock' freeOrionInterface
# module into memory.  For testing purposes the mock interface provides values for certain constants which various
# AI modules rely upon.  Since any later imports of the freeOrionInterface module will simply refer back the first-
# loaded module of the same name, code from the AI modules can then be imported and tested, provided that such
# AI code only relies upon constants from the freeOrionAIInterface (and specifically does not depend on any of the
# various dynamic galaxy-state queries provided by the 'true' freeOrionAIInterface).
# noinspection PyUnresolvedReferences
import freeOrionAIInterface
