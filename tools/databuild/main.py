
import os, os.path as path
from .exporters import load_exporters

"""
    DataBuild command line entry point
"""
def main(args):
    
    # Find exporter directory
    exp_dir = path.abspath(args[0])
    exp_dir = path.abspath(path.join(exp_dir, os.pardir, "exporters"))

    exporter_set = load_exporters([exp_dir])
    root_dir = args[1]

    # Load exporter classes
    for exp in exporter_set:
        print(exp)

    for dirpath, dirnames, filenames in os.walk(root_dir):
        for file in filenames:
            for exp in exporter_set:
                if exp.exportable(file):
                    print(exp.__name__, "=>", file)
    
    print(args)
