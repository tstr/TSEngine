"""
    Exporter module
"""

import sys
import importlib
import pkgutil
import os, os.path as path

class Exporter:
    """
        Exporter base interface
    """
    def __init__(self, source, context):
        self.context = context
        self.source = source

    @staticmethod
    def exportable(filename):
        return False

    """
        Fetch dependency info
    """
    def info(self, deps): pass

    """
        Run the exporter
    """
    def run(self): pass

    # Utility methods
    def rel_source(self):
        return os.path.relpath(self.source, self.context.datdir)


def load_exporters(paths):
    """
        Load a list of exporter classes from a given set of module paths
    """
    # Extend module search path
    sys.path.extend(paths)

    # Import every module in the given directory
    for importer, module_name, ispkg in pkgutil.iter_modules(path=paths):
        importlib.import_module(module_name)

    return Exporter.__subclasses__()

def get_exporters():
    """
        Returns list of available exporter classes
    """
    return Exporter.__subclasses__()

def get_exporter(classname):
    """
        Return an exporter class with the given name
    """
    for exp in get_exporters():
        if exp.__name__ == classname:
            return exp
    return None
