
import os
from .context import Context

"""
    DataBuild command line entry point
"""
def main(args):
    
    # Find exporter directory
    exp_dir = os.path.abspath(args[0])
    exp_dir = os.path.abspath(os.path.join(exp_dir, os.pardir, "exporters"))

    out_dir = os.getcwd()

    if args[1] == "--export":

        file = args[2]
        data_dir = args[3]

        ctx = Context(out_dir, exp_dir, data_dir)
        exp = ctx.find_exporter(file)
        exp.run()

    else:
        data_dir = args[1]

        print("data-dir:    ", data_dir)
        print("output-dir:  ", out_dir)
        print("exporter-dir:", exp_dir)

        ctx = Context(out_dir, exp_dir, data_dir)
        ctx.configure()
        ctx.build()
