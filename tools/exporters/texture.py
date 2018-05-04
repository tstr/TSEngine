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
        root, ext = path.splitext(self.rel_source())
        deps["outputs"] = [root + ".texture"]

    def run(self):
        print("converting:", self.rel_source())
        makedirs(path.dirname(self.rel_source()), exist_ok=True)
        root, ext = path.splitext(self.rel_source())
        tex.convert2D(self.source, root + ".texture")
