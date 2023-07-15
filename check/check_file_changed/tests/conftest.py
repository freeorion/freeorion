import sys
from pathlib import Path

this_dir = Path(__file__).parent
checks_dir = this_dir.parent.parent.parent.as_posix()
print(">>>>>", checks_dir)
sys.path.append(checks_dir)
