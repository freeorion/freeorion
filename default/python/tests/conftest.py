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

# This import put module to memory, after that we can import code form AI module without problem,
#  because module named freeOrionAIInterface is already in memory and imports in AI just use it.
# noinspection PyUnresolvedReferences
import freeOrionAIInterface
