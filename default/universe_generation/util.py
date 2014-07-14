import sys
import math


def distance(x1, y1, x2, y2):
    """
    Calculates linear distance between two coordinates
    """
    x_dist = float(x1 - x2)
    y_dist = float(y1 - y2)
    return math.sqrt((x_dist * x_dist) + (y_dist * y_dist))


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