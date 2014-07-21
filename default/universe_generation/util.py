import sys
import math


def distance(x1, y1, x2, y2):
    """
    Calculates linear distance between two coordinates
    """
    return math.hypot(float(x1) - float(x2), float(y1) - float(y2))


def load_string_list(file_name):
    """
    Reads a list of strings from a content file
    """
    try:
        with open(file_name, "r") as f:
            return [x.strip('" \t\r\n') for x in f]
    except:
        print "Unable to access", file_name
        print sys.exc_info()[1]
        return []