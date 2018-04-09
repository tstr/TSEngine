
import os, os.path as path
from .connectors import load_connectors

"""
    DataBuild command line entry point
"""
def main(args):
    
    # Find connector directory
    con_dir = path.abspath(args[0])
    con_dir = path.abspath(path.join(con_dir, os.pardir, "connectors"))

    # Load connectors
    for con in load_connectors([con_dir]):
        print(con)

    print(args)
    return 0
