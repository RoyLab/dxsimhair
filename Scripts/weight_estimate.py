from scipy.optimize import minimize
import coordinates as cd
import numpy as np
import metis_graph as mg
import cPickle as pkl
from progressbar import *

class SkinModel:

    def __init__(self, frames, graph):
        self.n_node = frames[0].n_hair
        self.n_frame = len(frames)

        self.weights = [None] * frames[0].n_hair
        self.data = frames
        self.graph = graph

        for i in range(frames[0].n_hair):
            self.weights[i] = [None, None]

    def estimate(self):
        print "estimating weights..."
        pbar = ProgressBar().start()
        for i in range(self.n_node):
            if self.graph.isGuideHair(i):
                continue

            Ci = self.collectCi(i)
            nw = len(Ci)
            cons = ({'type': 'eq',
                     'fun' : lambda x: np.sum(x)-1.0,
                     'jac' : lambda x: np.ones(len(x))
                     },
                     {'type': 'ineq',
                      'fun' : lambda x: x,
                      'jac' : lambda x: np.identity(len(x))
                      })

            res = minimize(SkinModel.evalError, [1.0/nw]*nw, args=(self, i, Ci),
             jac=SkinModel.evalDerive, options={'disp': False}, method='SLSQP', constraints=cons)

            map(lambda x: 0 if (x < 0.0) else x, res.x)
            # print res.x
            self.weights[i][0] = res.x
            self.weights[i][1] = Ci
            # if len(Ci) != 1 and abs(res.x[0]-res.x[1])<0.001:
                # import ipdb; ipdb.set_trace()
            pbar.update(100*i/(self.n_node-1))

        pbar.finish()

    def collectCi(self, s):
        eitr = mg.EdgeIterator(self.graph, s)
        groups = set([self.graph.hairGroup[s]])
        for ti in eitr:
            groups.add(self.graph.hairGroup[ti[0]])

        Ci = []
        for group in groups:
            Ci.append(self.graph.guide[group])
        return Ci

    @staticmethod
    def evalError(x, inst, s, Ci):
        t0 = inst.data[0].data[s], inst.data[0].particle_direction[s]
        n = len(x)
        sumAAT = np.matrix(np.zeros((n, n)))
        sumAs = np.matrix(np.zeros(n))
        sumSST = 00.

        for fn in range(inst.n_frame):
            A = []
            frame = inst.data[fn]
            tref = cd.rigid_trans_full(frame.rigid_motion, t0)
            treal = np.array([frame.data[s], frame.particle_direction[s]])
            treal.resize(6)
            for g in Ci:
                Bg = frame.particle_motions[g]
                # print g, Bg
                state = np.array(cd.point_trans(Bg, tref))
                state.resize(6)
                A.append(state)
            A = np.matrix(A)
            sumAAT += A*A.T
            sumAs += np.matrix(treal)*A.T
            sumSST += treal.dot(treal)

        inst.cacheMatrices(sumAAT, sumAs);
        return (x * sumAAT).dot(x) - 2 * sumAs.dot(x) + sumSST

    @staticmethod
    def evalDerive(x, inst, s, Ci):
        AAT, As = inst.retrieveMatrices()
        return (2 * AAT.dot(x) - 2 * As).A1

    def cacheMatrices(self, AAT, As):
        self.AAT = AAT
        self.As = As

    def retrieveMatrices(self):
        return self.AAT, self.As

    def dump(self, f):
        pkl.dump(self.weights, f, 2)

    def load(self, f):
        self.weights = pkl.load(f)

    def assessment(self):
        error = 0.0
        error0 = 0.0
        for i in range(self.n_node):
            if self.graph.isGuideHair(i):
                continue

            Ci = self.weights[i][1]
            nw = len(Ci)
            error0 += SkinModel.evalError([1.0/nw]*nw, self, i, Ci)
            error += SkinModel.evalError(self.weights[i][0], self, i, Ci)

        print "error decrease from %f to %f." % (error0, error)