"""
    Context module
"""

import sys
import os
import subprocess
import pickle
from .exporters import load_exporters, get_exporters, get_exporter

try:
    from ninja.ninja_syntax import Writer
    from ninja import _program as call_ninja
except ImportError as e:
    print(e)
    raise

class Context:
    def __init__(self, outdir, expdir, datadir):
        # Attributes
        self.outdir = outdir
        self.expdir = expdir
        self.datdir = datadir

        # Load exporter classes
        load_exporters([self.expdir])
        
        
    def check_index(self):
        """
            Fetch file index set and compare against data dir
            
            If it differs return a set of files in the data dir
            otherwise return None
        """
        
        # Traverse input directory
        file_set = set()
        for dirpath, dirnames, filenames in os.walk(self.datdir):
            for file in filenames:
                file_set.add(os.path.join(dirpath, file))
                
        idx_path = os.path.join(self.outdir, "index.dat")
        
        try:
            # Check index
            with open(idx_path, "rb") as idx_file:
                # Deserialize current index set
                idx_set = pickle.load(idx_file)
                assert(isinstance(idx_set, set))
                
                # Test if input directory has changed
                if idx_set == file_set:
                    # No changes
                    return None

        except (FileNotFoundError, UnpicklingError):
            # If there is an issue loading the index
            # Skip to update phase
            pass
        
        # Update index
        with open(idx_path, "wb") as idx_file:
            # Serialize new index set
            pickle.dump(file_set, idx_file)
        
        return file_set


    def configure(self):
        """
            Configure build system
        """
        
        data_files = self.check_index()
        
        # No need to reconfigure
        if not data_files:
            return

        print("configuring...")
        
        launcher_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../dbuild.py"))
        
        # Setup makefile
        with open(os.path.join(self.outdir, "build.ninja"), "w") as buildfile:
            n = Writer(buildfile)

            n.variable("OUTDIR", self.outdir)
            n.variable("DATDIR", self.datdir)
            n.variable("EXPDIR", self.expdir)
            n.variable("PY", sys.executable)
            n.variable("DBUILD", "$PY " + launcher_path)

            # Write build rules
            for cls in get_exporters():
                name = cls.__name__
                n.rule(name, cls.command())

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
        # Invoke ninja build system
        call_ninja(name="ninja",args=[])

    def find_exporter(self, filepath):
        """
            Find an exporter object for the given file
        """
        for exp in get_exporters():
            if exp.exportable(filepath):
                return exp(filepath, self)

        return None
        
    def export(self, exporter_name, filepath):
        """
            Export the given file
        """
        cls = get_exporter(exporter_name)
        cls(filepath, self).run()
