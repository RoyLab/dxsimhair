import nCache
import numpy as np
from coordinates import *
import cPickle as pkl

n_particle_per_strand = 25

def importFile(fileName, number=5):
    hk = Hooker()
    nCache.importFile(fileName, hk, number)
    return hk.get_data()

class Hooker:

    def __init__(self):
        self.frames = []
        self.cframe = None

    def data_hooker(self, name, sz, arr):
        # print sz, arr[0:5], len(arr), name
        self.cframe.loadIntoMemory(name, sz, arr)

    def new_frame(self):
        self.cframe = Frame()
        self.frames.append(self.cframe)

    def get_data(self):
        return self.frames


class Frame:

    def __init__(self):
        self.count = 0
        self.data = None # 2D array
        self.headData = None # 2D array

        self.n_headVertex = 0
        self.n_hair = 0
        self.n_particle = 0

        self.rigid_motion = None # matrix R, array t
        self.particle_motions = None # list of (matrix R, array t)
        self.reference = None

        self.hairspline = []
        self.particle_direction = None  # 2D array

    def loadIntoMemory(self, name, sz, data):
        if self.count == 0:
            # head data
            self.n_headVertex = int(sz)
            self.headData = np.array(data)
            self.headData.resize(len(data)/3, 3)

        elif self.count == 1:
            self.n_hair = int(data[0])
            self.n_particle = self.n_hair * n_particle_per_strand

        elif self.count == 3:
            # hair data, array, no need for any retrieval
            self.data = np.array(data)
            self.data.resize(self.n_particle, 3)

        self.count += 1

    def calcParticleMotionMatrices(self):
        ref = self.reference
        matrices = []
        for i in range(self.n_particle):
            trans = self.data[i] - (ref.data[i] + self.rigid_motion[1])
            rot = vector_rotation_3D((ref.particle_direction[i] * self.rigid_motion[0].T).A1, self.particle_direction[i])
            matrices.append((rot, trans))

        self.particle_motions = matrices

    def calcSelectedParticleMotionMatrices(self, reference, Ids):
        ref = reference
        self.reference = ref
        matrices = {}
        for i in Ids:
            trans = self.data[i] - (ref.data[i] + self.rigid_motion[1])
            rot = vector_rotation_3D((ref.particle_direction[i] * self.rigid_motion[0].T).A1, self.particle_direction[i])
            matrices[i] = (rot, trans)

        self.particle_motions = matrices

    def calcParticleDirections(self):
        from scipy import interpolate
        u_axis = np.linspace(0, 1, 25)

        directions = []
        for i in range(self.n_hair):
            begin = n_particle_per_strand*i
            end = n_particle_per_strand*(i+1)

            data = self.data[begin:end].T
            spline, u = interpolate.splprep(data, s=0)
            derive = interpolate.splev(u_axis, spline, der=1)

            derive = np.matrix(derive).T
            for j in range(n_particle_per_strand):
                directions.append(derive[j] / np.linalg.norm(derive[j]))
            self.hairspline.append(spline)

        self.particle_direction = np.array(directions)
        self.particle_direction.resize(self.n_particle, 3)


    def deviation(self, id0, id1):
        cur0 = self.data[id0], self.particle_direction[id0]
        cur1 = self.data[id1], self.particle_direction[id1]

        t0 = self.particle_motions[id0]
        t1 = self.particle_motions[id1]
        t = self.rigid_motion

        ref0 = rigid_trans(t, self.reference.data[id0]),\
            self.reference.particle_direction[id0] * t[0].T
        ref1 = rigid_trans(t, self.reference.data[id1]),\
            self.reference.particle_direction[id1] * t[0].T

        return squared_diff(point_trans(t0, ref1), cur1) + \
            squared_diff(point_trans(t1, ref0), cur0)

    def computeMotionMatrix(self, reference):
        self.reference = reference
        self.rigid_motion = rigid_transform_3D(matrix(reference.headData), matrix(self.headData))
        self.calcParticleDirections();
        self.calcParticleMotionMatrices();

    def clearMotionMatrix(self):
        del self.particle_motions
        del self.rigid_motion
        del self.particle_direction
        del self.hairspline

    def cacheInfo(self, f):
        pkl.dump((self.rigid_motion, self.particle_direction), f, 2)

    def loadCache(self, f):
        self.rigid_motion, self.particle_direction = \
            pkl.load(f)
