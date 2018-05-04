"""
    shader exporter
"""

import os.path as path
import databuild
import shaderlib

class Shader(databuild.Exporter):
    def exportable(filename):
        root, ext = path.splitext(filename)
        return ext in {".fx", ".hlsl"}

    def info(self, deps):
        root, ext = path.splitext(self.rel_source())
        deps["outputs"] = [root + ".shader"]

    def run(self):
        # Output dir of current source
        outdir = path.join(self.context.outdir, path.dirname(self.rel_source()))
        # Compile shader source
        print("compiling:", self.rel_source())
        if not shaderlib.compile([self.source], outdir):
            raise Exception("Unable to compile shader")
