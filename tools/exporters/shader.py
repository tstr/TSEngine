"""
    shader exporter
"""

import os.path as path
import shaderlib
from databuild import Exporter

class Shader(Exporter):
    def __init__(self, filename):
        super().__init__(filename)

    @staticmethod
    def exportable(filename):
        root, ext = path.splitext(filename)
        return ext in [".fx", ".hlsl", ]

    def outputs(self):
        return ""
    def depends(self):
        return []
    def depfile(self):
        return ""
    
    def export(self):
        pass
