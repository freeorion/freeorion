import os
import sys

current_dir = os.path.dirname(__file__)
common = os.path.join(current_dir, "..", "..", "..")
assert os.path.exists(common)
sys.path.append(common)

charting = os.path.join(current_dir, "..", "..")
assert os.path.exists(charting)
sys.path.append(charting)
