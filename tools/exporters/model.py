"""
    model exporter
"""

import os.path as path
import modellib
from databuild import Exporter

class Model(Exporter):
    
    builder = modellib.ModelBuilder()
    
    def exportable(filename):
        root, ext = path.splitext(filename)
        # find extension excluding dot
        return ext[1:] in Model.builder.supported_extensions()
