
import os
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

    if args[1] == "--export":

        # Args
        exporter_name = args[2]
        file = args[3]
        data_dir = args[4]

        ctx = Context(out_dir, exp_dir, data_dir)
        
        cls = get_exporter(exporter_name)
        cls(file, ctx).run()
        
    else:
        data_dir = args[1]

        print("data-dir:    ", data_dir)
        print("output-dir:  ", out_dir)
        print("exporter-dir:", exp_dir)

        ctx = Context(out_dir, exp_dir, data_dir)
        ctx.configure()
        ctx.build()
