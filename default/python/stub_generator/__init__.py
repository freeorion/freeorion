import os
from logging import debug

from stub_generator.constants import TYPE, NAME, ATTRS, DOC, PARENTS, CLASS_NAME, ENUM_PAIRS
from stub_generator.generate_stub import make_stub
from stub_generator.interface_inspector import get_module_info


def generate_stub(obj, instances, classes_to_ignore, path, dump=False):
    """
    Inspect interface and generate stub. Writes its logs to freeoriond.log.

    :param obj: main interface module (freeOrionAIInterface for AI)
    :param instances:  list of instances, required to get more detailed information about them
    :param classes_to_ignore: classes that should not to be reported when check for missed instances done.
                              this argument required because some classes present in interface
                              but have no methods, to get their instances.
    :param path: relative path from python folder
    """
    debug("\n\nStart generating skeleton for %s\n\n" % obj.__name__)
    python_folder_path = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))
    result_folder = os.path.join(python_folder_path, path)
    result_path = os.path.join(result_folder, '%s.pyi' % obj.__name__)
    make_stub(get_module_info(obj, instances, dump=dump), result_path, classes_to_ignore)
    debug("Skeleton written to %s" % result_path)
