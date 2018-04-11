"""
    Image exporter
"""

from shutil import copy
from os import makedirs
from os.path import splitext, dirname, join
from databuild import Exporter

class Image(Exporter):
    def exportable(filename):
        root, ext = splitext(filename)
        return ext in [".tga", ".png", ".jpg", ".JPG"]

    def info(self, deps):
        deps.outputs = [self.rel_source()]

    def run(self):
        # Output dir of current source
        outdir = join(self.context.outdir, dirname(self.rel_source()))

        makedirs(outdir, exist_ok=True)
        copy(src=self.source, dst=outdir)

