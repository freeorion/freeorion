"""
Config for tests inside this directory.
https://docs.pytest.org/en/2.7.3/plugins.html?highlight=re#working-with-plugins-and-conftest-files
"""

import os
import sys
from unittest.mock import MagicMock

this_dir = os.path.dirname(__file__)
sys.path.append(os.path.join(this_dir, "..", "..", "python", "AI/"))
sys.path.append(os.path.join(this_dir, "..", "..", "python"))


class aggression:
    beginner = 0
    turtle = 1
    cautious = 2
    typical = 3
    aggressive = 4
    maniacal = 5


def userString(x):
    return f"UserString {x}"


fo = MagicMock(name="freeOrionAIInterface")
fo.aggression = aggression
fo.userString = userString

sys.modules["freeOrionAIInterface"] = fo
