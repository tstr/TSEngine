
import os
import argparse
from .context import Context
from .exporters import get_exporter

"""
    DataBuild command line entry point
"""
def main(args):
    
    # Find exporter directory
    exp_dir = os.path.abspath(args[0])
    exp_dir = os.path.abspath(os.path.join(exp_dir, os.pardir, "exporters"))
    out_dir = os.getcwd()

    # Parse Command Line arguments
    parser = argparse.ArgumentParser(description = "Data build tool.")
    parser.add_argument("--build", help="build the given directory", nargs=1, metavar ="DATA_DIR")
    parser.add_argument("--export", help="export a file", nargs=3, metavar=("EXPORTER", "DATA_FILE", "DATA_DIR"))
    params = parser.parse_args(args[1:])
    
    # Export a single file
    if params.export:

        exporter_name = params.export[0]
        filepath = params.export[1]
        data_dir = params.export[2]

        ctx = Context(out_dir, exp_dir, data_dir)
        ctx.export(exporter_name, filepath)
        
    # Build a set of files
    elif params.build:

        data_dir = params.build[0]

        print("data-dir:    ", data_dir)
        print("output-dir:  ", out_dir)
        print("exporter-dir:", exp_dir)

        ctx = Context(out_dir, exp_dir, data_dir)
        ctx.configure()
        ctx.build()
