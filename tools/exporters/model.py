"""
    model exporter
"""

import modellib
import os.path as path
from databuild import Exporter

class Model(Exporter):
    
    builder = modellib.ModelBuilder()
    
    def exportable(filename):
        root, ext = path.splitext(filename)
        # find extension excluding dot
        return ext[1:] in Model.builder.supported_extensions()

    def info(self, deps):
        root, ext = path.splitext(self.rel_source())
        deps["outputs"] = [root + ".model"]

    def run(self):
        print("exporting:", self.rel_source())
        
        # Output dir of current file
        outdir = path.join(self.context.outdir, path.dirname(self.rel_source()))

        if not Model.builder.imp(self.source):
            print(Model.builder.error_string())
            raise Exception("Unable to read model")
        
        if not Model.builder.exp(outdir):
            print(Model.builder.error_string())
            raise Exception("Unable to write model")