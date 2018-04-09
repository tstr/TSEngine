
import os, os.path as path
from .exporters import load_exporters

"""
    DataBuild command line entry point
"""
def main(args):
    
    # Find exporter directory
    exp_dir = path.abspath(args[0])
    exp_dir = path.abspath(path.join(exp_dir, os.pardir, "exporters"))

    # Load exporter classes
    for exp in load_exporters([exp_dir]):
        print(exp)

    print(args)
