"""
    shader exporter
"""

from os.path import splitext, dirname, join
from databuild import Exporter
import shaderlib

class Shader(Exporter):
    def exportable(filename):
        root, ext = splitext(filename)
        return ext in [".fx", ".hlsl"]

    def info(self, deps):
        root, ext = splitext(self.rel_source())
        deps.outputs = [root + ".tsh"]

    def run(self):
        # Output dir of current source
        outdir = join(self.context.outdir, dirname(self.rel_source()))
        # Compile shader source
        print("compiling:", self.rel_source())
        if not shaderlib.compile([self.source], outdir):
            raise Exception("Unable to compile shader")
