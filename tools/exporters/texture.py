"""
    Texture exporter
"""

from shutil import copy
from os import makedirs
import os.path as path

import databuild as db
import texturelib as tex

class Texture(db.Exporter):
    def exportable(filename):
        root, ext = path.splitext(filename)
        return ext.lower() in {".tga", ".png", ".jpg" }

    def info(self, deps):
        deps["outputs"] = [self.rel_source()]

    def run(self):
        print("converting:", self.rel_source())
        makedirs(path.dirname(self.rel_source()), exist_ok=True)
        tex.convert2D(self.source, self.rel_source())
