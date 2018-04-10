"""
    model exporter
"""

import os.path as path
import modellib
from databuild import Exporter

class Model(Exporter):
    
    builder = modellib.ModelBuilder()
    
    def __init__(self, filename):
        super().__init__(filename)

    @staticmethod
    def exportable(filename):
        root, ext = path.splitext(filename)
        # find extension excluding dot
        return ext[1:] in Model.builder.supported_extensions()
