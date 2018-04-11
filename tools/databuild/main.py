
import os
from .context import Context

"""
    DataBuild command line entry point
"""
def main(args):
    
    # Find exporter directory
    exp_dir = os.path.abspath(args[0])
    exp_dir = os.path.abspath(os.path.join(exp_dir, os.pardir, "exporters"))

    data_dir = args[1]

    print("data-dir:    ", data_dir)
    print("output-dir:  ", os.getcwd())
    print("exporter-dir:", exp_dir)

    ctx = Context(os.getcwd(), exp_dir, data_dir)
    ctx.configure()

    print(args)
