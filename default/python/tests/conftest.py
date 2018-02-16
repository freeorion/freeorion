"""
Config for tests inside this directory.
https://docs.pytest.org/en/2.7.3/plugins.html?highlight=re#working-with-plugins-and-conftest-files
"""

import os
import sys

this_dir = os.path.dirname(__file__)

sys.path.append(os.path.join(this_dir, '..', '..', 'python', 'AI/'))
sys.path.append(os.path.join(this_dir, '..', '..', 'python', 'interface_mock', 'result'))
sys.path.append(os.path.join(this_dir, '..', '..', 'python'))
