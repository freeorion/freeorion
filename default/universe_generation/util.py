import sys


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