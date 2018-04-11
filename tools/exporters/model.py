"""
    model exporter
"""

from os.path import splitext, dirname, join
from databuild import Exporter
import modellib

class Model(Exporter):
    
    builder = modellib.ModelBuilder()
    
    def exportable(filename):
        root, ext = splitext(filename)
        # find extension excluding dot
        return ext[1:] in Model.builder.supported_extensions()

    def info(self, deps):
        root, ext = splitext(self.rel_source())
        deps["outputs"] = [root + ".tsm"]

    def run(self):
        # Output dir of current file
        outdir = join(self.context.outdir, dirname(self.rel_source()))

        print("exporting:", self.rel_source())

        if not Model.builder.imp(self.source):
            print(Model.builder.error_string())
            raise Exception("Unable to load model")
        
        if not Model.builder.exp(outdir):
            print(Model.builder.error_string())
            raise Exception("Unable to export model")