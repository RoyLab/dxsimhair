import numpy as np
from scipy.optimize import minimize


def evalError(x, inst, s, Ci):
    t0 = inst.data[0].data[s], inst.data[0].particle_direction[s]
    n = len(x)
    sumAAT = np.matrix(np.zeros((n, n)))
    sumAs = np.matrix(np.zeros(n))

    for fn in range(inst.n_frame):
        A = []
        frame = inst.data[fn]
        tref = cd.rigid_trans_full(frame.rigid_motion, t0)
        treal = np.array([frame.data[s], frame.particle_direction[s]])
        treal.resize(6)
        for g in Ci:
            Bg = frame.particle_motions[g]
            state = np.array(cd.point_trans(Bg, tref))
            state.resize(6)
            A.append(state)
        A = np.matrix(A)
        sumAAT += A*A.T
        sumAs += np.matrix(treal)*A.T

    inst.cacheMatrices(sumAAT, sumAs);
    return (x * sumAAT).dot(x) - 2 * sumAs.dot(x)
