# -*- coding: utf-8 -*-
import numpy as np
import matplotlib.pylab as pl
from scipy import interpolate
import ipdb
np.set_printoptions(suppress=True)

import rigid_transform as rtf

for i in range(20):
    a = np.random.rand(3)
    b = np.random.rand(3)
    # ipdb.set_trace()
    R = rtf.vector_rotation_3D_non_normalized(a, b)

    print a / np.linalg.norm(a) - b*R / np.linalg.norm(b*R)
    print b / np.linalg.norm(b) - a*R / np.linalg.norm(a*R)
