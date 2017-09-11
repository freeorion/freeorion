#!/usr/bin/env python2.7

import os
import sys
import pytest

this_dir = os.path.dirname(__file__)

# add path to AI
sys.path.append(os.path.join(this_dir, '..', '..', 'python', 'AI/'))
# path to freeOrionAIInterface mock
sys.path.append(os.path.join(this_dir, '..', '..', 'python', 'interface_mock', 'result'))
# add path to common tools
sys.path.append(os.path.join(this_dir, '..', '..', 'python'))

pytest.main(sys.argv[1:])
