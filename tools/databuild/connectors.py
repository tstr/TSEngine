"""
    Connector module
"""

import sys
import importlib
import pkgutil
import os, os.path as path

class Connector:
    """
        Connector base interface
    """
    pass

def load_connectors(paths):
    """
        Load a list of connector classes from a given set of module paths
    """
    # Extend module search path
    sys.path.extend(paths)

    # Import every module in the given directory
    for importer, module_name, ispkg in pkgutil.iter_modules(path=paths):
        importlib.import_module(module_name)

    return Connector.__subclasses__()

def get_connectors():
    """
        Returns list of available connector classes
    """
    return Connector.__subclasses__()