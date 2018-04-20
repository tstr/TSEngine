"""
    Context module
"""

import sys
import os
import subprocess
from .exporters import load_exporters, get_exporters

from ninja.ninja_syntax import Writer
from ninja import _program as call_ninja

class Context:
    def __init__(self, outdir, expdir, datadir):
        # Attributes
        self.outdir = outdir
        self.expdir = expdir
        self.datdir = datadir

        # Load exporter classes
        load_exporters([self.expdir])

    def configure(self):
        """
            Configure build system
        """
        data_files = []
        # Traverse root directory
        for dirpath, dirnames, filenames in os.walk(self.datdir):
            for file in filenames:
                data_files.append(os.path.join(dirpath, file))
        
        launcher_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../dbuild.py"))
        
        # Setup makefile
        with open(os.path.join(self.outdir, "build.ninja"), "w") as buildfile:
            n = Writer(buildfile)

            n.variable("OUTDIR", self.outdir)
            n.variable("DATDIR", self.datdir)
            n.variable("EXPDIR", self.expdir)
            n.variable("DBUILD", launcher_path)
            n.variable("PY", sys.executable)

            # Write build rules
            for cls in get_exporters():
                name = cls.__name__
                n.rule(name, "$PY $DBUILD --export %s $in $DATDIR" % name)

            # Build rule
            #n.rule("dbuild", "$PY $DBUILD --export $in $DATDIR")

            # For every data file
            for file in data_files:
                ex = self.find_exporter(file)
                if ex:
                    # Fetch file dependencies
                    deps = {"inputs":[file]}
                    ex.info(deps)
                    # Build statement
                    n.build(deps["outputs"], type(ex).__name__, deps["inputs"])

    def build(self):
        call_ninja(name="ninja",args=[])

    def find_exporter(self, filepath):
        """
            Find an exporter object for the given file
        """
        for exp in get_exporters():
            if exp.exportable(filepath):
                return exp(filepath, self)

        return None
