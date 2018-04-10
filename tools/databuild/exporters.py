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
    def __init__(self, filepath, context):
        self.context = context
        self.source = filepath

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
        return os.path.relpath(self.source, self.context.datadir)


class DependencyInfo:
    """
        Dependency info structure
    """
    def __init__(self):
        self.inputs = []
        self.outputs = []
        self.deps = []
        self.depfile = ""


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
