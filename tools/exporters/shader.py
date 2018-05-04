"""
    shader exporter
"""

import os.path as path
import databuild
from shaderlib import Source as ShaderSource

class Shader(databuild.Exporter):
    def exportable(filename):
        root, ext = path.splitext(filename)
        return ext in {".fx", ".hlsl"}

    def info(self, deps):
        root, ext = path.splitext(self.rel_source())
        deps["outputs"] = [root + ".shader"]
        
        srcfile = ShaderSource(self.source)
        deps["depends"] = srcfile.dependencies()

    def run(self):
    
        # Output dir of current source
        outdir = path.join(self.context.outdir, path.dirname(self.rel_source()))
        # Compile shader source
        print("compiling:", self.rel_source())
        
        # Load shader source file
        srcfile = ShaderSource(self.source)
        if srcfile.error_code() != ShaderSource.OK:
            raise Exception(srcfile.error_string())
        
        if srcfile.compile(outdir) != ShaderSource.OK:
            raise Exception(srcfile.error_string())
