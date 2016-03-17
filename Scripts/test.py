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

    v0 = a / np.linalg.norm(a)
    v1 = b / np.linalg.norm(b)

    v2 = (R * np.matrix(b).T).T
    v3 = (R * np.matrix(a).T).T

    v2 = v2 / np.linalg.norm(v2)
    v3 = v3 / np.linalg.norm(v3)

    print v0, v1
    print v2, v3
    print R * R.T
