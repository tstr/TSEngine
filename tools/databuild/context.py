"""
    Context module
"""

import os
from .exporters import load_exporters, get_exporters, DependencyInfo

class Context:
    def __init__(self, outdir, expdir, datadir):
        # Attributes
        self.outdir = outdir
        self.expdir = expdir
        self.datadir = datadir

        # Load exporter classes
        for exp in load_exporters([self.expdir]):
            print(exp)

    def process(self):
        """
            Process resources in the given data dir
        """
        # Traverse root directory
        for dirpath, dirnames, filenames in os.walk(self.datadir):
            for file in filenames:
                ex = self.find_exporter(os.path.join(dirpath, file))
                if ex:
                    # Fetch file dependencies
                    deps = DependencyInfo()
                    ex.info(deps)
                    # Test build
                    ex.run()

    def build(self):
        # todo: invoke ninja
        pass

    def find_exporter(self, filepath):
        """
            Find an exporter object for the given file
        """
        for exp in get_exporters():
            if exp.exportable(filepath):
                return exp(filepath, self)

        return None
