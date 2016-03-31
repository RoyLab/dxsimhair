from scipy.optimize import minimize
import coordinates as cd
import numpy as np
import metis_graph as mg
import cPickle as pkl
from progressbar import *

class SkinModel:

    def __init__(self, guide, frames, graph):
        self.n_node = frames[0].n_hair
        self.n_frame = len(frames)

        self.weights = [None] * frames[0].n_hair
        self.data = frames
        self.graph = graph

        for i in range(frames[0].n_hair):
            self.weights[i] = [None, None]

        self.initGroupNeighbourMap()

    def initGroupNeighbourMap(self):
        self.groupNeighMap = [None] * self.graph.n_group
        self.groupGuideMap = [None] * self.graph.n_group

        for i in range(self.graph.n_group):
            self.groupNeighMap[i] = set([])

        eItr = mg.UndirectedIterator(self.graph)
        for i, j, w in eItr:
            gi = self.graph.hairGroup[i]
            gj = self.graph.hairGroup[j]
            self.groupNeighMap[gi].add(gj)
            self.groupNeighMap[gj].add(gi)

        def findGuide(thiz, id):
            return thiz.graph.guide[id]

        for i in range(self.graph.n_group):
            n_neigh = len(self.groupNeighMap[i])
            self.groupGuideMap[i] = map(findGuide, [self]*n_neigh, self.groupNeighMap[i])

    def estimate(self):
        print "estimating weights..."
        self.error = 0.0
        self.error0 = 0.0
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
            self.weights[i][0] = res.x
            self.weights[i][1] = Ci
            pbar.update(100*i/(self.n_node-1))

            self.error0 += SkinModel.evalError([1.0/nw]*nw, self, i, Ci)
            self.error += SkinModel.evalError(self.weights[i][0], self, i, Ci)
        pbar.finish()
        print "error decrease from %f to %f." % (self.error0, self.error)

    def collectCi(self, s):
        ti = self.graph.hairGroup[s]
        return self.groupGuideMap[ti]

    @staticmethod
    def evalError(x, inst, s, Ci, idx = [-1]):
        npar = 25 # particle per strand
        if idx[0] != s:
            t0 = inst.data[0].data[s*npar:(s+1)*npar], inst.data[0].particle_direction[s*npar:(s+1)*npar]
            n = len(x)
            sumAAT = np.matrix(np.zeros((n, n)))
            sumAs = np.matrix(np.zeros(n))
            sumSST = 0.0

            for fn in range(inst.n_frame):
                A = []
                frame = inst.data[fn]
                tref = cd.rigid_trans_batch(frame.rigid_motion, t0)
                treal = np.array([frame.data[s*npar:(s+1)*npar], frame.particle_direction[s*npar:(s+1)*npar]])
                treal.resize(6*npar)
                for g in Ci:
                    Bg = frame.particle_motions[g]
                    state = np.array(cd.point_trans_batch(Bg, tref))
                    state.resize(6*npar)
                    A.append(state)
                A = np.matrix(A)
                sumAAT += A*A.T
                sumAs += np.matrix(treal)*A.T
                sumSST += treal.dot(treal)
            inst.cacheMatrices(sumAAT, sumAs, sumSST);
            idx[0] = s
        else:
            sumAAT, sumAs, sumSST = inst.retrieveMatrices()

        return (x * sumAAT).dot(x) - 2 * sumAs.dot(x) + sumSST

    @staticmethod
    def evalDerive(x, inst, s, Ci):
        AAT, As, SST = inst.retrieveMatrices()
        return (2 * AAT.dot(x) - 2 * As).A1

    def cacheMatrices(self, AAT, As, SST):
        self.AAT = AAT
        self.As = As
        self.SST = SST

    def retrieveMatrices(self):
        return self.AAT, self.As, self.SST

    def getResult(self):
        return self.weights

    def dump(self, f):
        pkl.dump(self.weights, f, 2)

    def load(self, f):
        self.weights = pkl.load(f)

    def assessment(self):
        self.error = 0.0
        self.error0 = 0.0
        for i in range(self.n_node):
            if self.graph.isGuideHair(i):
                continue

            Ci = self.weights[i][1]
            nw = len(Ci)
            self.error0 += SkinModel.evalError([1.0/nw]*nw, self, i, Ci)
            self.error += SkinModel.evalError(self.weights[i][0], self, i, Ci)

        print "error decrease from %f to %f." % (self.error0, self.error)
